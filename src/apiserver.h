#ifndef APISERVER_H
#define APISERVER_H

#include <QObject>
#include <QHttpServer>
#include <QNetworkReply>
#include <QString>
#include <QHostAddress>
#include <QSslKey>
#include <QSslCertificate>
#include <QTimer>
#include <QMap>
#include <QMutex>

class ConfigManager;

class ApiServer : public QObject
{
    Q_OBJECT

public:
    explicit ApiServer(QObject *parent = nullptr);
    ~ApiServer();

    // Listen with improved security (default to localhost only)
    bool listen(int port, const QHostAddress &address = QHostAddress::LocalHost);
    
    // Listen on HTTP port for HTTPS redirects (when TLS is enabled)
    bool listenHttpRedirect(int httpPort, int httpsPort);
    
    // Enable TLS/HTTPS
    bool enableTls(const QString &certPath, const QString &keyPath, const QString &keyPassphrase = QString());
    
    // Configure CORS
    void setCorsEnabled(bool enabled, const QStringList &allowedOrigins = {"*"});
    
    // Configure rate limiting
    void setRateLimit(int maxRequestsPerMinute);
    
    // Set problem detail base URL
    void setProblemBaseUrl(const QString &baseUrl);
    
    // Set configuration manager
    void setConfig(ConfigManager *config);

private:
    QHttpServer *m_server;
    QHttpServer *m_redirectServer;  // Server for HTTP redirects
    bool m_corsEnabled;
    QStringList m_corsAllowedOrigins;
    int m_rateLimit;
    QMap<QString, int> m_clientRequests;
    QMutex m_rateLimitMutex;
    QString m_problemBaseUrl;
    bool m_tlsEnabled;
    ConfigManager *m_config;
    int m_httpsPort;  // HTTPS port for redirects
    
    void setupRoutes();
    void setupErrorHandler();
    void setupSecurityHeaders();
    void addSecurityHeaders(QHttpServerResponse &response);
    void addCorsHeaders(QHttpServerResponse &response);
    QHttpServerResponse handleException(const std::exception &e, const QHttpServerRequest &request);
    bool isRateLimited(const QString &clientIp);
    QHttpServerResponse createRateLimitedResponse(const QString &clientIp);
    void resetRateLimits();
    void setupHttpsRedirect(int httpPort, int httpsPort);
    QString getServerHostname() const;
};

#endif // APISERVER_H