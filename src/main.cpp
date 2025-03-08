#include <QCoreApplication>
#include <QCommandLineParser>
#include <QHostAddress>
#include <iostream>
#include "apiserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("Qt6 Web API Example");
    QCoreApplication::setApplicationVersion("1.0.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("A simple Qt6 C++ web API that returns 'Hello World' with RFC 7807 error handling");
    parser.addHelpOption();
    parser.addVersionOption();

    // Port option
    QCommandLineOption portOption(QStringList() << "p" << "port",
                                  "Port to listen on (default: 8080)",
                                  "port", "8080");
    parser.addOption(portOption);
    
    // Host address option
    QCommandLineOption hostOption(QStringList() << "a" << "address",
                                 "Address to bind to (default: localhost). Use '0.0.0.0' for all interfaces.",
                                 "address", "localhost");
    parser.addOption(hostOption);
    
    // TLS options
    QCommandLineOption tlsOption(QStringList() << "tls",
                                "Enable TLS/HTTPS");
    parser.addOption(tlsOption);
    
    QCommandLineOption certOption(QStringList() << "cert",
                                 "Path to TLS certificate file (PEM format)",
                                 "cert");
    parser.addOption(certOption);
    
    QCommandLineOption keyOption(QStringList() << "key",
                                "Path to TLS private key file (PEM format)",
                                "key");
    parser.addOption(keyOption);
    
    QCommandLineOption passphraseOption(QStringList() << "passphrase",
                                      "Passphrase for TLS private key (if encrypted)",
                                      "passphrase", "");
    parser.addOption(passphraseOption);
    
    // CORS options
    QCommandLineOption corsOption(QStringList() << "cors",
                                 "Enable CORS support");
    parser.addOption(corsOption);
    
    QCommandLineOption corsOriginsOption(QStringList() << "cors-origins",
                                       "Comma-separated list of allowed CORS origins (default: *)",
                                       "origins", "*");
    parser.addOption(corsOriginsOption);
    
    // Rate limiting option
    QCommandLineOption rateLimitOption(QStringList() << "rate-limit",
                                     "Maximum requests per minute per client IP (default: 100, 0 to disable)",
                                     "limit", "100");
    parser.addOption(rateLimitOption);
    
    // Problem detail base URL option
    QCommandLineOption problemBaseUrlOption(QStringList() << "problem-base-url",
                                          "Base URL for problem detail type URIs",
                                          "url", "https://problemdetails.example.com/problems");
    parser.addOption(problemBaseUrlOption);

    parser.process(app);

    int port = parser.value(portOption).toInt();
    QString hostStr = parser.value(hostOption);
    QHostAddress host = (hostStr == "localhost") ? QHostAddress::LocalHost :
                        (hostStr == "0.0.0.0") ? QHostAddress::Any : 
                        QHostAddress(hostStr);
    
    bool enableTls = parser.isSet(tlsOption);
    QString certPath = parser.value(certOption);
    QString keyPath = parser.value(keyOption);
    QString passphrase = parser.value(passphraseOption);
    
    bool enableCors = parser.isSet(corsOption);
    QStringList corsOrigins = parser.value(corsOriginsOption).split(",", Qt::SkipEmptyParts);
    
    int rateLimit = parser.value(rateLimitOption).toInt();
    QString problemBaseUrl = parser.value(problemBaseUrlOption);

    // Create and configure the API server
    ApiServer server;
    
    // Configure problem detail base URL
    server.setProblemBaseUrl(problemBaseUrl);
    
    // Configure rate limiting
    server.setRateLimit(rateLimit);
    
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
        std::cerr << "Failed to start server on " << hostStr.toStdString() << ":" << port << std::endl;
        return 1;
    }

    std::cout << "Server running at http" << (enableTls ? "s" : "") << "://" 
              << (hostStr == "0.0.0.0" ? "localhost" : hostStr.toStdString()) << ":" << port << std::endl;
    std::cout << "Press Ctrl+C to quit" << std::endl;

    // Display configured security options
    if (enableCors) {
        std::cout << "CORS enabled with origins: " 
                  << (corsOrigins.isEmpty() ? "*" : parser.value(corsOriginsOption).toStdString()) << std::endl;
    }
    
    if (rateLimit > 0) {
        std::cout << "Rate limiting: " << rateLimit << " requests per minute per client" << std::endl;
    } else {
        std::cout << "Rate limiting: disabled" << std::endl;
    }

    return app.exec();
}