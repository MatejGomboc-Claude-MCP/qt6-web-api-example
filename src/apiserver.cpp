#include "apiserver.h"
#include "problemdetail.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QString>

ApiServer::ApiServer(QObject *parent)
    : QObject(parent), m_server(new QHttpServer(this))
{
    setupRoutes();
    setupErrorHandler();
}

ApiServer::~ApiServer()
{
}

bool ApiServer::listen(int port)
{
    const quint16 actualPort = m_server->listen(QHostAddress::Any, port);
    return actualPort != 0;
}

void ApiServer::setupRoutes()
{
    // Root route returns Hello World
    m_server->route("/", [] {
        return "Hello World";
    });

    // API routes
    m_server->route("/api", [] {
        return QJsonObject{{"message", "Hello World"}};
    });

    // Example route that triggers a 404 error
    m_server->route("/api/not-found", [] {
        // This demonstrates how to manually trigger a problem detail error
        ProblemDetail problem(404);
        problem.setTitle("Resource Not Found");
        problem.setDetail("The requested resource does not exist");
        problem.setInstance("/api/not-found");
        
        return problem.toJsonResponse();
    });

    // Example route that triggers a 500 error
    m_server->route("/api/error", [] {
        ProblemDetail problem(500);
        problem.setTitle("Internal Server Error");
        problem.setDetail("An unexpected error occurred");
        problem.setInstance("/api/error");
        problem.addExtension("server_info", "Qt6 Web API Example");
        
        return problem.toJsonResponse();
    });
}

void ApiServer::setupErrorHandler()
{
    // Handle 404 errors for any undefined routes
    m_server->handleUnmatchedRoute([](const QHttpServerRequest &request) {
        ProblemDetail problem(404);
        problem.setTitle("Not Found");
        problem.setDetail(QString("The requested resource '%1' was not found").arg(request.url().path()));
        problem.setInstance(request.url().path());
        
        return problem.toJsonResponse();
    });
}