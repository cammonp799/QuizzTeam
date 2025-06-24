#include "networkmanager.h"
#include <QDebug>

NetworkManager::NetworkManager(QObject *parent) : QObject(parent), server(nullptr), socket(nullptr) {}

void NetworkManager::startHost(int port) {
    server = new QTcpServer(this);
    connect(server, &QTcpServer::newConnection, [=]() {
        QTcpSocket *client = server->nextPendingConnection();
        qDebug() << "Client connecté";
    });
    server->listen(QHostAddress::Any, port);
}

void NetworkManager::connectToHost(const QString &host, int port) {
    socket = new QTcpSocket(this);
    socket->connectToHost(host, port);
    if (socket->waitForConnected()) {
        qDebug() << "Connecté au serveur";
    }
}
