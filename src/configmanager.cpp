#include "configmanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QHostAddress>

ConfigManager::ConfigManager()
{
    setDefaults();
}

bool ConfigManager::loadConfig(const QString &configPath)
{
    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // Failed to open config file - we'll use defaults
        return false;
    }
    
    QByteArray configData = file.readAll();
    file.close();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(configData, &error);
    
    if (error.error != QJsonParseError::NoError) {
        // JSON parsing error
        return false;
    }
    
    if (!doc.isObject()) {
        // Root must be an object
        return false;
    }
    
    m_config = doc.object();
    m_configPath = configPath;
    
    return true;
}

bool ConfigManager::processCommandLine()
{
    QCommandLineParser parser;
    parser.setApplicationDescription("Qt6 Web API Example");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // Config file option
    QCommandLineOption configOption(QStringList() << "c" << "config",
                                  "Path to configuration file",
                                  "config", "config.json");
    parser.addOption(configOption);
    
    // Server options
    QCommandLineOption portOption(QStringList() << "p" << "port",
                                "Port to listen on", "port");
    parser.addOption(portOption);
    
    QCommandLineOption addressOption(QStringList() << "a" << "address",
                                   "Address to bind to", "address");
    parser.addOption(addressOption);
    
    // HTTP to HTTPS redirect options
    QCommandLineOption httpRedirectOption(QStringList() << "http-redirect",
                                        "Enable HTTP to HTTPS redirect", "enable", "false");
    parser.addOption(httpRedirectOption);
    
    QCommandLineOption httpPortOption(QStringList() << "http-port",
                                    "HTTP port for redirects", "port", "80");
    parser.addOption(httpPortOption);
    
    // Rate limiting options
    QCommandLineOption rateLimitOption(QStringList() << "rate-limit",
                                     "Enable rate limiting", "enable", "true");
    parser.addOption(rateLimitOption);
    
    QCommandLineOption maxRequestsOption(QStringList() << "max-requests",
                                       "Maximum requests per minute", "count");
    parser.addOption(maxRequestsOption);
    
    // CORS options
    QCommandLineOption corsOption(QStringList() << "cors",
                               "Enable CORS", "enable", "false");
    parser.addOption(corsOption);
    
    QCommandLineOption corsOriginsOption(QStringList() << "cors-origins",
                                      "Comma-separated allowed origins", "origins");
    parser.addOption(corsOriginsOption);
    
    // TLS options
    QCommandLineOption tlsOption(QStringList() << "tls",
                               "Enable TLS/HTTPS", "enable", "false");
    parser.addOption(tlsOption);
    
    QCommandLineOption certOption(QStringList() << "cert",
                               "Path to TLS certificate", "cert");
    parser.addOption(certOption);
    
    QCommandLineOption keyOption(QStringList() << "key",
                              "Path to TLS private key", "key");
    parser.addOption(keyOption);
    
    // Problem details options
    QCommandLineOption problemBaseUrlOption(QStringList() << "problem-base-url",
                                         "Base URL for problem details", "url");
    parser.addOption(problemBaseUrlOption);
    
    parser.process(*QCoreApplication::instance());
    
    // Load config file if specified
    if (parser.isSet(configOption)) {
        QString configPath = parser.value(configOption);
        if (!loadConfig(configPath)) {
            return false;
        }
    }
    
    // Override with command-line options if specified
    QJsonObject serverObj = m_config["server"].toObject();
    if (parser.isSet(portOption)) {
        serverObj["port"] = parser.value(portOption).toInt();
    }
    
    if (parser.isSet(addressOption)) {
        serverObj["address"] = parser.value(addressOption);
    }
    
    // HTTP to HTTPS redirect overrides
    if (parser.isSet(httpRedirectOption) || parser.isSet(httpPortOption)) {
        QJsonObject httpRedirectObj = serverObj["httpRedirect"].toObject();
        
        if (parser.isSet(httpRedirectOption)) {
            httpRedirectObj["enabled"] = (parser.value(httpRedirectOption).toLower() == "true");
        }
        
        if (parser.isSet(httpPortOption)) {
            httpRedirectObj["httpPort"] = parser.value(httpPortOption).toInt();
        }
        
        serverObj["httpRedirect"] = httpRedirectObj;
    }
    
    m_config["server"] = serverObj;
    
    // Rate limiting overrides
    if (parser.isSet(rateLimitOption) || parser.isSet(maxRequestsOption)) {
        QJsonObject securityObj = m_config["security"].toObject();
        QJsonObject rateLimitObj = securityObj["rateLimit"].toObject();
        
        if (parser.isSet(rateLimitOption)) {
            rateLimitObj["enabled"] = (parser.value(rateLimitOption).toLower() == "true");
        }
        
        if (parser.isSet(maxRequestsOption)) {
            rateLimitObj["maxRequestsPerMinute"] = parser.value(maxRequestsOption).toInt();
        }
        
        securityObj["rateLimit"] = rateLimitObj;
        m_config["security"] = securityObj;
    }
    
    // CORS overrides
    if (parser.isSet(corsOption) || parser.isSet(corsOriginsOption)) {
        QJsonObject securityObj = m_config["security"].toObject();
        QJsonObject corsObj = securityObj["cors"].toObject();
        
        if (parser.isSet(corsOption)) {
            corsObj["enabled"] = (parser.value(corsOption).toLower() == "true");
        }
        
        if (parser.isSet(corsOriginsOption)) {
            QJsonArray originsArray;
            const QStringList origins = parser.value(corsOriginsOption).split(",", Qt::SkipEmptyParts);
            for (const QString &origin : origins) {
                originsArray.append(origin.trimmed());
            }
            corsObj["allowedOrigins"] = originsArray;
        }
        
        securityObj["cors"] = corsObj;
        m_config["security"] = securityObj;
    }
    
    // TLS overrides
    if (parser.isSet(tlsOption) || parser.isSet(certOption) || parser.isSet(keyOption)) {
        QJsonObject securityObj = m_config["security"].toObject();
        QJsonObject tlsObj = securityObj["tls"].toObject();
        
        if (parser.isSet(tlsOption)) {
            tlsObj["enabled"] = (parser.value(tlsOption).toLower() == "true");
        }
        
        if (parser.isSet(certOption)) {
            tlsObj["certificatePath"] = parser.value(certOption);
        }
        
        if (parser.isSet(keyOption)) {
            tlsObj["keyPath"] = parser.value(keyOption);
        }
        
        securityObj["tls"] = tlsObj;
        m_config["security"] = securityObj;
    }
    
    // Problem details overrides
    if (parser.isSet(problemBaseUrlOption)) {
        QJsonObject problemObj = m_config["problemDetails"].toObject();
        problemObj["baseUrl"] = parser.value(problemBaseUrlOption);
        m_config["problemDetails"] = problemObj;
    }
    
    return true;
}

int ConfigManager::getPort() const
{
    return getInt({"server", "port"}, 8080);
}

QHostAddress ConfigManager::getAddress() const
{
    QString address = getString({"server", "address"}, "localhost");
    
    if (address == "localhost") {
        return QHostAddress::LocalHost;
    } else if (address == "any" || address == "0.0.0.0") {
        return QHostAddress::Any;
    } else {
        return QHostAddress(address);
    }
}

int ConfigManager::getWorkers() const
{
    return getInt({"server", "workers"}, 4);
}

bool ConfigManager::isHttpRedirectEnabled() const
{
    return getBool({"server", "httpRedirect", "enabled"}, false);
}

int ConfigManager::getHttpPort() const
{
    return getInt({"server", "httpRedirect", "httpPort"}, 80);
}

bool ConfigManager::isRateLimitEnabled() const
{
    return getBool({"security", "rateLimit", "enabled"}, true);
}

int ConfigManager::getMaxRequestsPerMinute() const
{
    return getInt({"security", "rateLimit", "maxRequestsPerMinute"}, 100);
}

QStringList ConfigManager::getRateLimitIpWhitelist() const
{
    return getStringList({"security", "rateLimit", "ipWhitelist"}, {"127.0.0.1", "::1"});
}

bool ConfigManager::isCorsEnabled() const
{
    return getBool({"security", "cors", "enabled"}, false);
}

QStringList ConfigManager::getAllowedOrigins() const
{
    return getStringList({"security", "cors", "allowedOrigins"}, {"*"});
}

QStringList ConfigManager::getAllowedMethods() const
{
    return getStringList({"security", "cors", "allowedMethods"}, {"GET", "POST", "OPTIONS"});
}

QStringList ConfigManager::getAllowedHeaders() const
{
    return getStringList({"security", "cors", "allowedHeaders"}, {"Content-Type", "Authorization"});
}

int ConfigManager::getCorsMaxAge() const
{
    return getInt({"security", "cors", "maxAge"}, 86400);
}

bool ConfigManager::isTlsEnabled() const
{
    return getBool({"security", "tls", "enabled"}, false);
}

QString ConfigManager::getCertificatePath() const
{
    return getString({"security", "tls", "certificatePath"}, "");
}

QString ConfigManager::getKeyPath() const
{
    return getString({"security", "tls", "keyPath"}, "");
}

QString ConfigManager::getPassphrase() const
{
    return getString({"security", "tls", "passphrase"}, "");
}

QString ConfigManager::getContentTypeOptions() const
{
    return getString({"security", "headers", "contentTypeOptions"}, "nosniff");
}

QString ConfigManager::getFrameOptions() const
{
    return getString({"security", "headers", "frameOptions"}, "DENY");
}

QString ConfigManager::getContentSecurityPolicy() const
{
    return getString({"security", "headers", "contentSecurityPolicy"}, "default-src 'self'");
}

QString ConfigManager::getPermissionsPolicy() const
{
    return getString({"security", "headers", "permissionsPolicy"}, "geolocation=(), camera=(), microphone=()");
}

QString ConfigManager::getReferrerPolicy() const
{
    return getString({"security", "headers", "referrerPolicy"}, "strict-origin-when-cross-origin");
}

QString ConfigManager::getXssProtection() const
{
    return getString({"security", "headers", "xssProtection"}, "1; mode=block");
}

int ConfigManager::getHstsMaxAge() const
{
    return getInt({"security", "headers", "hstsMaxAge"}, 31536000);
}

bool ConfigManager::getHstsIncludeSubdomains() const
{
    return getBool({"security", "headers", "hstsIncludeSubdomains"}, true);
}

QString ConfigManager::getCacheControl() const
{
    return getString({"security", "headers", "cacheControl"}, "no-store, max-age=0");
}

QString ConfigManager::getClearSiteData() const
{
    return getString({"security", "headers", "clearSiteData"}, "");
}

QString ConfigManager::getCrossOriginEmbedderPolicy() const
{
    return getString({"security", "headers", "crossOriginEmbedderPolicy"}, "require-corp");
}

QString ConfigManager::getCrossOriginOpenerPolicy() const
{
    return getString({"security", "headers", "crossOriginOpenerPolicy"}, "same-origin");
}

QString ConfigManager::getCrossOriginResourcePolicy() const
{
    return getString({"security", "headers", "crossOriginResourcePolicy"}, "same-origin");
}

QString ConfigManager::getProblemBaseUrl() const
{
    return getString({"problemDetails", "baseUrl"}, "https://problemdetails.example.com/problems");
}

bool ConfigManager::includeDebugInfo() const
{
    return getBool({"problemDetails", "includeDebugInfo"}, false);
}

QString ConfigManager::getContactEmail() const
{
    return getString({"problemDetails", "contactEmail"}, "");
}

QString ConfigManager::getLogLevel() const
{
    return getString({"logging", "level"}, "info");
}

QString ConfigManager::getLogFile() const
{
    return getString({"logging", "file"}, "");
}

bool ConfigManager::isConsoleLoggingEnabled() const
{
    return getBool({"logging", "console"}, true);
}

bool ConfigManager::includeTimestamp() const
{
    return getBool({"logging", "includeTimestamp"}, true);
}

void ConfigManager::setDefaults()
{
    // Create default configuration
    QJsonObject serverObj;
    serverObj["port"] = 8080;
    serverObj["address"] = "localhost";
    serverObj["workers"] = 4;
    
    QJsonObject httpRedirectObj;
    httpRedirectObj["enabled"] = false;
    httpRedirectObj["httpPort"] = 80;
    serverObj["httpRedirect"] = httpRedirectObj;
    
    QJsonObject rateLimitObj;
    rateLimitObj["enabled"] = true;
    rateLimitObj["maxRequestsPerMinute"] = 100;
    QJsonArray ipWhitelistArray;
    ipWhitelistArray.append("127.0.0.1");
    ipWhitelistArray.append("::1");
    rateLimitObj["ipWhitelist"] = ipWhitelistArray;
    
    QJsonObject corsObj;
    corsObj["enabled"] = false;
    QJsonArray originsArray;
    originsArray.append("*");
    corsObj["allowedOrigins"] = originsArray;
    QJsonArray methodsArray;
    methodsArray.append("GET");
    methodsArray.append("POST");
    methodsArray.append("OPTIONS");
    corsObj["allowedMethods"] = methodsArray;
    QJsonArray headersArray;
    headersArray.append("Content-Type");
    headersArray.append("Authorization");
    corsObj["allowedHeaders"] = headersArray;
    corsObj["maxAge"] = 86400;
    
    QJsonObject tlsObj;
    tlsObj["enabled"] = false;
    tlsObj["certificatePath"] = "";
    tlsObj["keyPath"] = "";
    tlsObj["passphrase"] = "";
    
    QJsonObject headersObj;
    headersObj["contentTypeOptions"] = "nosniff";
    headersObj["frameOptions"] = "DENY";
    headersObj["contentSecurityPolicy"] = "default-src 'self'";
    headersObj["permissionsPolicy"] = "geolocation=(), camera=(), microphone=()";
    headersObj["referrerPolicy"] = "strict-origin-when-cross-origin";
    headersObj["xssProtection"] = "1; mode=block";
    headersObj["hstsMaxAge"] = 31536000;
    headersObj["hstsIncludeSubdomains"] = true;
    headersObj["cacheControl"] = "no-store, max-age=0";
    headersObj["clearSiteData"] = "";
    headersObj["crossOriginEmbedderPolicy"] = "require-corp";
    headersObj["crossOriginOpenerPolicy"] = "same-origin";
    headersObj["crossOriginResourcePolicy"] = "same-origin";
    
    QJsonObject securityObj;
    securityObj["rateLimit"] = rateLimitObj;
    securityObj["cors"] = corsObj;
    securityObj["tls"] = tlsObj;
    securityObj["headers"] = headersObj;
    
    QJsonObject problemDetailsObj;
    problemDetailsObj["baseUrl"] = "https://problemdetails.example.com/problems";
    problemDetailsObj["includeDebugInfo"] = false;
    problemDetailsObj["contactEmail"] = "";
    
    QJsonObject loggingObj;
    loggingObj["level"] = "info";
    loggingObj["file"] = "";
    loggingObj["console"] = true;
    loggingObj["includeTimestamp"] = true;
    
    QJsonObject configObj;
    configObj["server"] = serverObj;
    configObj["security"] = securityObj;
    configObj["problemDetails"] = problemDetailsObj;
    configObj["logging"] = loggingObj;
    
    m_config = configObj;
}

QString ConfigManager::getString(const QStringList &path, const QString &defaultValue) const
{
    QJsonObject current = m_config;
    
    for (int i = 0; i < path.size() - 1; ++i) {
        if (!current.contains(path[i]) || !current[path[i]].isObject()) {
            return defaultValue;
        }
        current = current[path[i]].toObject();
    }
    
    if (!current.contains(path.last()) || !current[path.last()].isString()) {
        return defaultValue;
    }
    
    return current[path.last()].toString();
}

int ConfigManager::getInt(const QStringList &path, int defaultValue) const
{
    QJsonObject current = m_config;
    
    for (int i = 0; i < path.size() - 1; ++i) {
        if (!current.contains(path[i]) || !current[path[i]].isObject()) {
            return defaultValue;
        }
        current = current[path[i]].toObject();
    }
    
    if (!current.contains(path.last()) || !current[path.last()].isDouble()) {
        return defaultValue;
    }
    
    return current[path.last()].toInt();
}

bool ConfigManager::getBool(const QStringList &path, bool defaultValue) const
{
    QJsonObject current = m_config;
    
    for (int i = 0; i < path.size() - 1; ++i) {
        if (!current.contains(path[i]) || !current[path[i]].isObject()) {
            return defaultValue;
        }
        current = current[path[i]].toObject();
    }
    
    if (!current.contains(path.last()) || !current[path.last()].isBool()) {
        return defaultValue;
    }
    
    return current[path.last()].toBool();
}

QStringList ConfigManager::getStringList(const QStringList &path, const QStringList &defaultValue) const
{
    QJsonObject current = m_config;
    
    for (int i = 0; i < path.size() - 1; ++i) {
        if (!current.contains(path[i]) || !current[path[i]].isObject()) {
            return defaultValue;
        }
        current = current[path[i]].toObject();
    }
    
    if (!current.contains(path.last()) || !current[path.last()].isArray()) {
        return defaultValue;
    }
    
    QJsonArray array = current[path.last()].toArray();
    QStringList result;
    
    for (const QJsonValue &value : array) {
        if (value.isString()) {
            result.append(value.toString());
        }
    }
    
    return result.isEmpty() ? defaultValue : result;
}