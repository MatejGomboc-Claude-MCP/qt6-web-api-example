#include "apiserver.h"
#include "problemdetail.h"
#include "configmanager.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QString>
#include <QFile>
#include <QSslConfiguration>
#include <QSslKey>
#include <QSslCertificate>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QDateTime>
#include <QMutexLocker>
#include <stdexcept>

ApiServer::ApiServer(QObject *parent)
    : QObject(parent), 
      m_server(new QHttpServer(this)),
      m_corsEnabled(false),
      m_corsAllowedOrigins({"*"}),
      m_rateLimit(100), // Default: 100 requests per minute
      m_problemBaseUrl("https://problemdetails.example.com/problems"),
      m_tlsEnabled(false),
      m_config(new ConfigManager())
{
    setupRoutes();
    setupErrorHandler();
    setupSecurityHeaders();
    
    // Reset rate limits every minute
    QTimer *rateLimitTimer = new QTimer(this);
    connect(rateLimitTimer, &QTimer::timeout, this, &ApiServer::resetRateLimits);
    rateLimitTimer->start(60000); // 1 minute
}

ApiServer::~ApiServer()
{
    delete m_config;
}

bool ApiServer::listen(int port, const QHostAddress &address)
{
    const quint16 actualPort = m_server->listen(address, port);
    return actualPort != 0;
}

bool ApiServer::enableTls(const QString &certPath, const QString &keyPath, const QString &keyPassphrase)
{
    QFile certFile(certPath);
    QFile keyFile(keyPath);
    
    if (!certFile.open(QIODevice::ReadOnly) || !keyFile.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QSslCertificate certificate(&certFile, QSsl::Pem);
    QSslKey key(&keyFile, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, keyPassphrase.toUtf8());
    
    certFile.close();
    keyFile.close();
    
    if (certificate.isNull() || key.isNull()) {
        return false;
    }
    
    QSslConfiguration sslConfig;
    sslConfig.setLocalCertificate(certificate);
    sslConfig.setPrivateKey(key);
    sslConfig.setProtocol(QSsl::TlsV1_3OrLater);
    
    m_server->sslSetup(sslConfig);
    m_tlsEnabled = true;
    
    return true;
}

void ApiServer::setCorsEnabled(bool enabled, const QStringList &allowedOrigins)
{
    m_corsEnabled = enabled;
    m_corsAllowedOrigins = allowedOrigins;
}

void ApiServer::setRateLimit(int maxRequestsPerMinute)
{
    m_rateLimit = maxRequestsPerMinute;
}

void ApiServer::setProblemBaseUrl(const QString &baseUrl)
{
    m_problemBaseUrl = baseUrl;
    ProblemDetail::setDefaultBaseUrl(baseUrl);
}

void ApiServer::setConfig(ConfigManager *config)
{
    if (m_config) {
        delete m_config;
    }
    m_config = config;
}

void ApiServer::setupRoutes()
{
    // Wrap all routes with rate limiting and exception handling
    m_server->route("/", [this](const QHttpServerRequest &request) {
        try {
            // Check rate limiting
            if (isRateLimited(request.remoteAddress().toString())) {
                return createRateLimitedResponse(request.remoteAddress().toString());
            }
            
            // Add CORS headers if enabled
            QHttpServerResponse response("Hello World");
            
            // Add CORS headers if enabled
            addCorsHeaders(response);
            
            // Add OWASP recommended security headers
            addSecurityHeaders(response);
            
            return response;
        } catch (const std::exception &e) {
            return handleException(e, request);
        }
    });

    // API routes with JSON response
    m_server->route("/api", [this](const QHttpServerRequest &request) {
        try {
            // Check rate limiting
            if (isRateLimited(request.remoteAddress().toString())) {
                return createRateLimitedResponse(request.remoteAddress().toString());
            }
            
            QJsonObject jsonObject{{"message", "Hello World"}};
            QHttpServerResponse response(jsonObject);
            
            // Add CORS headers if enabled
            addCorsHeaders(response);
            
            // Add OWASP recommended security headers
            addSecurityHeaders(response);
            
            return response;
        } catch (const std::exception &e) {
            return handleException(e, request);
        }
    });

    // Example route that triggers a 404 error
    m_server->route("/api/not-found", [this](const QHttpServerRequest &request) {
        try {
            // Check rate limiting
            if (isRateLimited(request.remoteAddress().toString())) {
                return createRateLimitedResponse(request.remoteAddress().toString());
            }
            
            // This demonstrates how to manually trigger a problem detail error
            ProblemDetail problem(404);
            problem.setTitle("Resource Not Found");
            problem.setDetail("The requested resource does not exist");
            problem.setInstance("/api/not-found");
            
            return problem.toJsonResponse();
        } catch (const std::exception &e) {
            return handleException(e, request);
        }
    });

    // Example route that triggers a 500 error
    m_server->route("/api/error", [this](const QHttpServerRequest &request) {
        try {
            // Check rate limiting
            if (isRateLimited(request.remoteAddress().toString())) {
                return createRateLimitedResponse(request.remoteAddress().toString());
            }
            
            ProblemDetail problem(500);
            problem.setTitle("Internal Server Error");
            problem.setDetail("An unexpected error occurred");
            problem.setInstance("/api/error");
            problem.addExtension("server_info", "Qt6 Web API Example");
            
            return problem.toJsonResponse();
        } catch (const std::exception &e) {
            return handleException(e, request);
        }
    });
    
    // Handle OPTIONS requests for CORS
    m_server->route("*", QHttpServerRequest::Method::Options, [this](const QHttpServerRequest &request) {
        QHttpServerResponse response("");
        
        // Add CORS headers if enabled
        addCorsHeaders(response);
        
        // Add OWASP recommended security headers
        addSecurityHeaders(response);
        
        return response;
    });
}

void ApiServer::setupErrorHandler()
{
    // Handle 404 errors for any undefined routes
    m_server->handleUnmatchedRoute([this](const QHttpServerRequest &request) {
        try {
            // Check rate limiting
            if (isRateLimited(request.remoteAddress().toString())) {
                return createRateLimitedResponse(request.remoteAddress().toString());
            }
            
            ProblemDetail problem(404);
            problem.setTitle("Not Found");
            problem.setDetail(QString("The requested resource '%1' was not found").arg(request.url().path()));
            problem.setInstance(request.url().path());
            
            auto response = problem.toJsonResponse();
            
            // Add CORS headers if enabled
            addCorsHeaders(response);
            
            // Add OWASP recommended security headers
            addSecurityHeaders(response);
            
            return response;
        } catch (const std::exception &e) {
            return handleException(e, request);
        }
    });
}

void ApiServer::setupSecurityHeaders()
{
    // Set security headers for all responses
    m_server->afterRequest([this](QHttpServerResponse &&response) {
        // Add OWASP recommended security headers
        addSecurityHeaders(response);
        
        return std::move(response);
    });
}

void ApiServer::addSecurityHeaders(QHttpServerResponse &response)
{
    if (!m_config) {
        return;
    }
    
    // Add basic security headers
    response.setHeader("X-Content-Type-Options", m_config->getContentTypeOptions().toUtf8());
    response.setHeader("X-Frame-Options", m_config->getFrameOptions().toUtf8());
    response.setHeader("Content-Security-Policy", m_config->getContentSecurityPolicy().toUtf8());
    
    // Add additional OWASP recommended headers
    if (!m_config->getPermissionsPolicy().isEmpty()) {
        response.setHeader("Permissions-Policy", m_config->getPermissionsPolicy().toUtf8());
    }
    
    if (!m_config->getReferrerPolicy().isEmpty()) {
        response.setHeader("Referrer-Policy", m_config->getReferrerPolicy().toUtf8());
    }
    
    if (!m_config->getXssProtection().isEmpty()) {
        response.setHeader("X-XSS-Protection", m_config->getXssProtection().toUtf8());
    }
    
    if (!m_config->getCacheControl().isEmpty()) {
        response.setHeader("Cache-Control", m_config->getCacheControl().toUtf8());
    }
    
    if (!m_config->getClearSiteData().isEmpty()) {
        response.setHeader("Clear-Site-Data", m_config->getClearSiteData().toUtf8());
    }
    
    if (!m_config->getCrossOriginEmbedderPolicy().isEmpty()) {
        response.setHeader("Cross-Origin-Embedder-Policy", m_config->getCrossOriginEmbedderPolicy().toUtf8());
    }
    
    if (!m_config->getCrossOriginOpenerPolicy().isEmpty()) {
        response.setHeader("Cross-Origin-Opener-Policy", m_config->getCrossOriginOpenerPolicy().toUtf8());
    }
    
    if (!m_config->getCrossOriginResourcePolicy().isEmpty()) {
        response.setHeader("Cross-Origin-Resource-Policy", m_config->getCrossOriginResourcePolicy().toUtf8());
    }
    
    // Only add HSTS header if TLS is enabled
    if (m_tlsEnabled && m_config->getHstsMaxAge() > 0) {
        QString hstsValue = QString("max-age=%1").arg(m_config->getHstsMaxAge());
        if (m_config->getHstsIncludeSubdomains()) {
            hstsValue += "; includeSubDomains";
        }
        response.setHeader("Strict-Transport-Security", hstsValue.toUtf8());
    }
}

void ApiServer::addCorsHeaders(QHttpServerResponse &response)
{
    if (m_corsEnabled) {
        for (const auto &origin : m_corsAllowedOrigins) {
            response.setHeader("Access-Control-Allow-Origin", origin.toUtf8());
        }
        
        QString allowedMethods;
        for (const auto &method : m_config->getAllowedMethods()) {
            if (!allowedMethods.isEmpty()) {
                allowedMethods += ", ";
            }
            allowedMethods += method;
        }
        
        QString allowedHeaders;
        for (const auto &header : m_config->getAllowedHeaders()) {
            if (!allowedHeaders.isEmpty()) {
                allowedHeaders += ", ";
            }
            allowedHeaders += header;
        }
        
        response.setHeader("Access-Control-Allow-Methods", allowedMethods.toUtf8());
        response.setHeader("Access-Control-Allow-Headers", allowedHeaders.toUtf8());
        response.setHeader("Access-Control-Max-Age", QString::number(m_config->getCorsMaxAge()).toUtf8());
    }
}

QHttpServerResponse ApiServer::handleException(const std::exception &e, const QHttpServerRequest &request)
{
    ProblemDetail problem(500);
    problem.setTitle("Internal Server Error");
    problem.setDetail(QString("An unexpected error occurred: %1").arg(e.what()));
    problem.setInstance(request.url().path());
    
    auto response = problem.toJsonResponse();
    
    // Add CORS headers if enabled
    addCorsHeaders(response);
    
    // Add OWASP recommended security headers
    addSecurityHeaders(response);
    
    return response;
}

bool ApiServer::isRateLimited(const QString &clientIp)
{
    // Skip rate limiting if disabled
    if (m_rateLimit <= 0) {
        return false;
    }
    
    // Check whitelist
    if (m_config && m_config->getRateLimitIpWhitelist().contains(clientIp)) {
        return false;
    }
    
    QMutexLocker locker(&m_rateLimitMutex);
    
    // Increment request count for this client
    m_clientRequests[clientIp]++;
    
    // Check if client has exceeded rate limit
    return m_clientRequests[clientIp] > m_rateLimit;
}

QHttpServerResponse ApiServer::createRateLimitedResponse(const QString &clientIp)
{
    ProblemDetail problem(429);
    problem.setTitle("Too Many Requests");
    problem.setDetail(QString("You have exceeded the rate limit of %1 requests per minute").arg(m_rateLimit));
    problem.setInstance(QString("/rate-limit/%1").arg(clientIp));
    problem.addExtension("retryAfter", 60); // Try again in 60 seconds
    
    auto response = problem.toJsonResponse();
    response.setHeader("Retry-After", "60");
    
    // Add CORS headers if enabled
    addCorsHeaders(response);
    
    // Add OWASP recommended security headers
    addSecurityHeaders(response);
    
    return response;
}

void ApiServer::resetRateLimits()
{
    QMutexLocker locker(&m_rateLimitMutex);
    m_clientRequests.clear();
}