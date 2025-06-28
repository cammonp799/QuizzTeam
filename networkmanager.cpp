
// ---------- networkmanager.cpp ----------
#include "networkmanager.h"
#include <QHostAddress>
#include <QRandomGenerator>
#include <QJsonParseError>
#include <QDebug>

static const char TERMINATOR = '\n'; // delimite chaque message JSON

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
    if (server)
        stopServer();

    server = new QTcpServer(this);
    connect(server, &QTcpServer::newConnection, this, &NetworkManager::onNewConnection);

    if (!server->listen(QHostAddress::Any, port)) {
        emit connectionError("Cannot start server: " + server->errorString());
        server->deleteLater();
        server = nullptr;
        return false;
    }

    serverMode = true;
    emit serverStarted(server->serverPort());
    return true;
}

void NetworkManager::stopServer()
{
    if (!server)
        return;

    for (auto it = connectedClients.begin(); it != connectedClients.end(); ++it) {
        it.key()->disconnectFromHost();
        it.key()->deleteLater();
    }
    connectedClients.clear();
    buffers.clear();

    server->close();
    server->deleteLater();
    server = nullptr;
    serverMode = false;

    emit serverStopped();
}

quint16 NetworkManager::getServerPort() const
{
    return (server && server->isListening()) ? server->serverPort() : 0;
}

void NetworkManager::connectToHost(const QString &hostAddress, quint16 port)
{
    if (clientSocket)
        disconnectFromHost();

    clientSocket = new QTcpSocket(this);
    connect(clientSocket, &QTcpSocket::connected, this, &NetworkManager::connectedToHost);
    connect(clientSocket, &QTcpSocket::disconnected, this, &NetworkManager::disconnectedFromHost);
    connect(clientSocket, &QTcpSocket::readyRead, this, &NetworkManager::onDataReceived);
    connect(clientSocket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            [this](QAbstractSocket::SocketError) {
                emit connectionError(clientSocket->errorString());
            });

    serverMode = false;
    clientSocket->connectToHost(hostAddress, port);
}

void NetworkManager::disconnectFromHost()
{
    if (!clientSocket)
        return;

    clientSocket->disconnectFromHost();
    clientSocket->deleteLater();
    clientSocket = nullptr;
    clientBuffer.clear();
}

void NetworkManager::sendMessage(const QJsonObject &message)
{
    if (serverMode) {
        broadcastMessage(message);
    } else if (clientSocket && clientSocket->state() == QTcpSocket::ConnectedState) {
        sendToClient(clientSocket, message);
    }
}

void NetworkManager::broadcastMessage(const QJsonObject &message)
{
    if (!serverMode)
        return;

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
    return serverMode ? (server && server->isListening())
                      : (clientSocket && clientSocket->state() == QTcpSocket::ConnectedState);
}

QStringList NetworkManager::getConnectedClients() const
{
    return connectedClients.values();
}

void NetworkManager::onNewConnection()
{
    while (server->hasPendingConnections()) {
        QTcpSocket *socket = server->nextPendingConnection();
        QString clientId = generateClientId();

        connectedClients[socket] = clientId;
        buffers[socket] = QByteArray();

        connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onDataReceived);
        connect(socket, &QTcpSocket::disconnected, this, &NetworkManager::onClientDisconnected);

        emit clientConnected(clientId);
    }
}

void NetworkManager::onClientDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket)
        return;

    QString clientId = connectedClients.take(socket);
    buffers.remove(socket);

    socket->deleteLater();

    emit clientDisconnected(clientId);
}

void NetworkManager::onDataReceived()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket)
        return;

    QByteArray data = socket->readAll();

    if (serverMode) {
        QByteArray &buffer = buffers[socket];
        buffer.append(data);
        processBuffer(buffer, socket);
    } else {
        clientBuffer.append(data);
        processBuffer(clientBuffer, nullptr);
    }
}

void NetworkManager::processBuffer(QByteArray &buffer, QTcpSocket *originSocket)
{
    int idx;
    while ((idx = buffer.indexOf(TERMINATOR)) != -1) {
        QByteArray line = buffer.left(idx);
        buffer.remove(0, idx + 1); // supprime y compris le terminator

        if (line.isEmpty())
            continue;

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(line, &err);
        if (err.error != QJsonParseError::NoError) {
            qDebug() << "JSON parse error:" << err.errorString();
            continue;
        }

        QJsonObject obj = doc.object();

        if (serverMode) {
            QString clientId = connectedClients.value(originSocket);
            handleClientMessage(originSocket, obj);
            emit messageReceived(obj, clientId);
        } else {
            emit messageReceived(obj, "server");
        }
    }
}

void NetworkManager::handleClientMessage(QTcpSocket *socket, const QJsonObject &message)
{
    Q_UNUSED(socket)
    Q_UNUSED(message)
    // Ici, l’hôte peut filtrer ou valider les messages entrants si nécessaire
}

QString NetworkManager::generateClientId()
{
    return QString("client_%1").arg(QRandomGenerator::global()->bounded(1000, 9999));
}

void NetworkManager::sendToClient(QTcpSocket *socket, const QJsonObject &message)
{
    if (!socket || socket->state() != QTcpSocket::ConnectedState)
        return;

    QJsonDocument doc(message);
    QByteArray payload = doc.toJson(QJsonDocument::Compact);
    payload.append(TERMINATOR);

    socket->write(payload);
    socket->flush();
}
