#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class NetworkManager : public QObject {
    Q_OBJECT

public:
    explicit NetworkManager(QObject *parent = nullptr);
    void startHost(int port);
    void connectToHost(const QString &host, int port);

private:
    QTcpServer *server;
    QTcpSocket *socket;
};

#endif // NETWORKMANAGER_H
