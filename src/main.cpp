#include <QCoreApplication>
#include <QCommandLineParser>
#include <QHostAddress>
#include <iostream>
#include "apiserver.h"
#include "configmanager.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("Qt6 Web API Example");
    QCoreApplication::setApplicationVersion("1.0.0");

    // Create and initialize the configuration manager
    ConfigManager *config = new ConfigManager();
    
    // Process command line arguments
    if (!config->processCommandLine()) {
        std::cerr << "Error processing command line arguments" << std::endl;
        delete config;
        return 1;
    }
    
    // Get configuration values
    int port = config->getPort();
    QHostAddress host = config->getAddress();
    bool enableTls = config->isTlsEnabled();
    QString certPath = config->getCertificatePath();
    QString keyPath = config->getKeyPath();
    QString passphrase = config->getPassphrase();
    bool enableCors = config->isCorsEnabled();
    QStringList corsOrigins = config->getAllowedOrigins();
    int rateLimit = config->getMaxRequestsPerMinute();
    QString problemBaseUrl = config->getProblemBaseUrl();

    // Create and configure the API server
    ApiServer server;
    
    // Set the configuration manager
    server.setConfig(config);
    
    // Configure problem detail base URL
    server.setProblemBaseUrl(problemBaseUrl);
    
    // Configure rate limiting if enabled
    if (config->isRateLimitEnabled()) {
        server.setRateLimit(rateLimit);
    } else {
        server.setRateLimit(0); // Disable rate limiting
    }
    
    // Configure CORS if enabled
    if (enableCors) {
        server.setCorsEnabled(true, corsOrigins);
    }
    
    // Configure TLS if enabled
    if (enableTls) {
        if (certPath.isEmpty() || keyPath.isEmpty()) {
            std::cerr << "Error: TLS requires certificate and key files" << std::endl;
            return 1;
        }
        
        if (!server.enableTls(certPath, keyPath, passphrase)) {
            std::cerr << "Error: Failed to set up TLS with the provided certificate and key" << std::endl;
            return 1;
        }
        
        std::cout << "TLS enabled with certificate: " << certPath.toStdString() << std::endl;
    }
    
    // Start listening for connections
    if (!server.listen(port, host)) {
        std::cerr << "Failed to start server on " 
                  << (host == QHostAddress::LocalHost ? "localhost" : host.toString().toStdString())
                  << ":" << port << std::endl;
        return 1;
    }

    // Display server information
    std::cout << "Server running at http" << (enableTls ? "s" : "") << "://" 
              << (host == QHostAddress::Any ? "0.0.0.0" : 
                 (host == QHostAddress::LocalHost ? "localhost" : host.toString().toStdString()))
              << ":" << port << std::endl;
    std::cout << "Press Ctrl+C to quit" << std::endl;

    // Display configured security options
    if (enableCors) {
        std::cout << "CORS enabled with origins: ";
        for (const auto &origin : corsOrigins) {
            std::cout << origin.toStdString() << " ";
        }
        std::cout << std::endl;
    }
    
    if (config->isRateLimitEnabled()) {
        std::cout << "Rate limiting: " << rateLimit << " requests per minute per client" << std::endl;
    } else {
        std::cout << "Rate limiting: disabled" << std::endl;
    }
    
    if (!problemBaseUrl.isEmpty()) {
        std::cout << "Problem detail base URL: " << problemBaseUrl.toStdString() << std::endl;
    }
    
    std::cout << "OWASP recommended security headers: enabled" << std::endl;

    return app.exec();
}