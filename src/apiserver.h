#ifndef APISERVER_H
#define APISERVER_H

#include <QObject>
#include <QHttpServer>
#include <QNetworkReply>
#include <QString>

class ApiServer : public QObject
{
    Q_OBJECT

public:
    explicit ApiServer(QObject *parent = nullptr);
    ~ApiServer();

    bool listen(int port);

private:
    QHttpServer *m_server;

    void setupRoutes();
    void setupErrorHandler();
};

#endif // APISERVER_H