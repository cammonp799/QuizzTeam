#include "networkmanager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QDebug>

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent), server(nullptr), clientSocket(nullptr), serverMode(false)
{
}

NetworkManager::~NetworkManager()
{
    stopServer();
    disconnectFromHost();
}

bool NetworkManager::startServer(quint16 port)
{
    if (server) {
        stopServer();
    }
    
    server = new QTcpServer(this);
    connect(server, &QTcpServer::newConnection, this, &NetworkManager::onNewConnection);
    
    if (!server->listen(QHostAddress::Any, port)) {
        emit connectionError("Cannot start server: " + server->errorString());
        delete server;
        server = nullptr;
        return false;
    }
    
    serverMode = true;
    emit serverStarted(server->serverPort());
    return true;
}

void NetworkManager::stopServer()
{
    if (server) {
        // Disconnect all clients
        for (auto it = connectedClients.begin(); it != connectedClients.end(); ++it) {
            it.key()->disconnectFromHost();
        }
        connectedClients.clear();
        
        server->close();
        delete server;
        server = nullptr;
        serverMode = false;
        emit serverStopped();
    }
}

quint16 NetworkManager::getServerPort() const
{
    if (server && server->isListening()) {
        return server->serverPort();
    }
    return 0;
}

void NetworkManager::connectToHost(const QString& hostAddress, quint16 port)
{
    if (clientSocket) {
        disconnectFromHost();
    }
    
    clientSocket = new QTcpSocket(this);
    connect(clientSocket, &QTcpSocket::connected, this, &NetworkManager::connectedToHost);
    connect(clientSocket, &QTcpSocket::disconnected, this, &NetworkManager::disconnectedFromHost);
    connect(clientSocket, &QTcpSocket::readyRead, this, &NetworkManager::onDataReceived);
    connect(clientSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            [this](QAbstractSocket::SocketError error) {
                emit connectionError("Connection error: " + clientSocket->errorString());
            });
    
    serverMode = false;
    clientSocket->connectToHost(hostAddress, port);
}

void NetworkManager::disconnectFromHost()
{
    if (clientSocket) {
        clientSocket->disconnectFromHost();
        delete clientSocket;
        clientSocket = nullptr;
    }
}

void NetworkManager::sendMessage(const QJsonObject& message)
{
    if (serverMode) {
        broadcastMessage(message);
    } else if (clientSocket && clientSocket->state() == QTcpSocket::ConnectedState) {
        sendToClient(clientSocket, message);
    }
}

void NetworkManager::broadcastMessage(const QJsonObject& message)
{
    if (!serverMode) return;
    
    for (auto it = connectedClients.begin(); it != connectedClients.end(); ++it) {
        sendToClient(it.key(), message);
    }
}

bool NetworkManager::isServer() const
{
    return serverMode;
}

bool NetworkManager::isConnected() const
{
    if (serverMode) {
        return server && server->isListening();
    } else {
        return clientSocket && clientSocket->state() == QTcpSocket::ConnectedState;
    }
}

QStringList NetworkManager::getConnectedClients() const
{
    return connectedClients.values();
}

void NetworkManager::onNewConnection()
{
    if (!server) return;
    
    while (server->hasPendingConnections()) {
        QTcpSocket* socket = server->nextPendingConnection();
        QString clientId = generateClientId();
        
        connectedClients[socket] = clientId;
        
        connect(socket, &QTcpSocket::disconnected, this, &NetworkManager::onClientDisconnected);
        connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onDataReceived);
        
        emit clientConnected(clientId);
    }
}

void NetworkManager::onClientDisconnected()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    
    QString clientId = connectedClients.value(socket, "");
    connectedClients.remove(socket);
    
    socket->deleteLater();
    
    if (!clientId.isEmpty()) {
        emit clientDisconnected(clientId);
    }
}

void NetworkManager::onDataReceived()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    
    QByteArray data = socket->readAll();
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        qDebug() << "JSON parse error:" << error.errorString();
        return;
    }
    
    QJsonObject message = doc.object();
    
    if (serverMode) {
        QString clientId = connectedClients.value(socket, "");
        handleClientMessage(socket, message);
        emit messageReceived(message, clientId);
    } else {
        emit messageReceived(message, "server");
    }
}

void NetworkManager::handleClientMessage(QTcpSocket* socket, const QJsonObject& message)
{
    // Handle specific client messages if needed
    // For now, just forward to the game logic
}

QString NetworkManager::generateClientId()
{
    return QString("client_%1").arg(QRandomGenerator::global()->bounded(1000, 9999));
}

void NetworkManager::sendToClient(QTcpSocket* socket, const QJsonObject& message)
{
    if (!socket || socket->state() != QTcpSocket::ConnectedState) {
        return;
    }
    
    QJsonDocument doc(message);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    socket->write(data);
    socket->flush();
}