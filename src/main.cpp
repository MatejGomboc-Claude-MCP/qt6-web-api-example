#include <QCoreApplication>
#include <QCommandLineParser>
#include <iostream>
#include "apiserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("Qt6 Web API Example");
    QCoreApplication::setApplicationVersion("1.0.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("A simple Qt6 C++ web API that returns 'Hello World'");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption portOption(QStringList() << "p" << "port",
                                  "Port to listen on (default: 8080)",
                                  "port", "8080");
    parser.addOption(portOption);

    parser.process(app);

    int port = parser.value(portOption).toInt();

    ApiServer server;
    if (!server.listen(port)) {
        std::cerr << "Failed to start server on port " << port << std::endl;
        return 1;
    }

    std::cout << "Server running at http://localhost:" << port << std::endl;
    std::cout << "Press Ctrl+C to quit" << std::endl;

    return app.exec();
}