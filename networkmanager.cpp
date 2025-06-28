#include "networkmanager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QDebug>
#include <QHostAddress>
#include <QNetworkInterface>

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent), server(nullptr), clientSocket(nullptr), 
      connectionTimer(nullptr), serverMode(false)
{
    connectionTimer = new QTimer(this);
    connectionTimer->setSingleShot(true);
    connect(connectionTimer, &QTimer::timeout, this, &NetworkManager::onConnectionTimeout);
}

NetworkManager::~NetworkManager()
{
    cleanupConnections();
}

bool NetworkManager::startServer(quint16 port)
{
    if (server) {
        stopServer();
    }
    
    server = new QTcpServer(this);
    connect(server, &QTcpServer::newConnection, this, &NetworkManager::onNewConnection);
    
    // Essayer d'abord le port spécifié, sinon un port automatique
    if (!server->listen(QHostAddress::Any, port)) {
        qDebug() << "Impossible d'écouter sur le port" << port << ":" << server->errorString();
        
        // Essayer avec un port automatique
        if (!server->listen(QHostAddress::Any, 0)) {
            emit connectionError("Impossible de démarrer le serveur: " + server->errorString());
            delete server;
            server = nullptr;
            return false;
        }
    }
    
    serverMode = true;
    quint16 actualPort = server->serverPort();
    QString address = getServerAddress();
    
    qDebug() << "Serveur démarré sur" << address << ":" << actualPort;
    emit serverStarted(actualPort, address);
    return true;
}

void NetworkManager::stopServer()
{
    if (server) {
        qDebug() << "Arrêt du serveur...";
        
        // Déconnecter tous les clients proprement
        for (auto it = connectedClients.begin(); it != connectedClients.end(); ++it) {
            QTcpSocket* socket = it.key();
            socket->disconnectFromHost();
            if (socket->state() != QTcpSocket::UnconnectedState) {
                socket->waitForDisconnected(1000);
            }
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

QString NetworkManager::getServerAddress() const
{
    // Obtenir l'adresse IP locale
    QList<QHostAddress> addresses = QNetworkInterface::allAddresses();
    
    for (const QHostAddress& address : addresses) {
        if (address != QHostAddress::LocalHost && 
            address.toIPv4Address() && 
            !address.isLoopback()) {
            return address.toString();
        }
    }
    
    return QHostAddress(QHostAddress::LocalHost).toString();
}

void NetworkManager::connectToHost(const QString& hostAddress, quint16 port)
{
    if (clientSocket) {
        disconnectFromHost();
    }
    
    clientSocket = new QTcpSocket(this);
    
    // Connecter les signaux
    connect(clientSocket, &QTcpSocket::connected, this, &NetworkManager::connectedToHost);
    connect(clientSocket, &QTcpSocket::disconnected, this, &NetworkManager::disconnectedFromHost);
    connect(clientSocket, &QTcpSocket::readyRead, this, &NetworkManager::onDataReceived);
    
    // Gestion des erreurs avec la nouvelle syntaxe Qt5+
    connect(clientSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            [this](QAbstractSocket::SocketError error) {
                Q_UNUSED(error)
                QString errorMsg = QString("Erreur de connexion: %1").arg(clientSocket->errorString());
                qDebug() << errorMsg;
                emit connectionError(errorMsg);
                connectionTimer->stop();
            });
    
    serverMode = false;
    
    // Démarrer un timer de timeout pour la connexion
    connectionTimer->start(5000); // 5 secondes timeout
    
    qDebug() << "Tentative de connexion à" << hostAddress << ":" << port;
    clientSocket->connectToHost(hostAddress, port);
}

void NetworkManager::disconnectFromHost()
{
    if (clientSocket) {
        connectionTimer->stop();
        clientSocket->disconnectFromHost();
        
        if (clientSocket->state() != QTcpSocket::UnconnectedState) {
            clientSocket->waitForDisconnected(1000);
        }
        
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

int NetworkManager::getClientCount() const
{
    return connectedClients.size();
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
        
        qDebug() << "Nouveau client connecté:" << clientId << "depuis" << socket->peerAddress().toString();
        emit clientConnected(clientId);
    }
}

void NetworkManager::onClientDisconnected()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    
    QString clientId = connectedClients.value(socket, "");
    connectedClients.remove(socket);
    
    qDebug() << "Client déconnecté:" << clientId;
    socket->deleteLater();
    
    if (!clientId.isEmpty()) {
        emit clientDisconnected(clientId);
    }
}

void NetworkManager::onDataReceived()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    
    while (socket->canReadLine()) {
        QByteArray data = socket->readLine();
        
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);
        
        if (error.error != QJsonParseError::NoError) {
            qDebug() << "Erreur JSON:" << error.errorString();
            continue;
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
}

void NetworkManager::onConnectionTimeout()
{
    if (clientSocket && clientSocket->state() == QTcpSocket::ConnectingState) {
        clientSocket->abort();
        emit connectionError("Timeout: Impossible de se connecter au serveur");
    }
}

void NetworkManager::handleClientMessage(QTcpSocket* socket, const QJsonObject& message)
{
    // Traiter les messages spécifiques du client si nécessaire
    Q_UNUSED(socket)
    Q_UNUSED(message)
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
    QByteArray data = doc.toJson(QJsonDocument::Compact) + "\n"; // Ajouter un saut de ligne
    
    qint64 bytesWritten = socket->write(data);
    if (bytesWritten == -1) {
        qDebug() << "Erreur lors de l'envoi:" << socket->errorString();
    } else {
        socket->flush();
    }
}

void NetworkManager::cleanupConnections()
{
    stopServer();
    disconnectFromHost();
    
    if (connectionTimer) {
        connectionTimer->stop();
    }
}