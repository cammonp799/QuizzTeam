// ================================
//  NetworkManager UPDATED (copy‑paste ready)
// ================================

// ---------- networkmanager.h ----------
#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMap>

class NetworkManager : public QObject
{
    Q_OBJECT
public:
    explicit NetworkManager(QObject *parent = nullptr);
    ~NetworkManager();

    // --- Host side ---
    bool startServer(quint16 port = 12345);   // démarre l’hôte (12345 par défaut)
    void stopServer();
    quint16 getServerPort() const;

    // --- Client side ---
    void connectToHost(const QString &hostAddress, quint16 port = 12345);
    void disconnectFromHost();

    // --- Messaging ---
    void sendMessage(const QJsonObject &message);   // automatique (broadcast côté hôte)
    void broadcastMessage(const QJsonObject &message);

    // --- State helpers ---
    bool isServer() const;
    bool isConnected() const;
    QStringList getConnectedClients() const;

signals:
    void serverStarted(quint16 port);
    void serverStopped();
    void clientConnected(const QString &clientId);
    void clientDisconnected(const QString &clientId);
    void connectedToHost();
    void disconnectedFromHost();
    void messageReceived(const QJsonObject &message, const QString &senderId);
    void connectionError(const QString &error);

private slots:
    void onNewConnection();
    void onClientDisconnected();
    void onDataReceived();

private:
    // Core sockets
    QTcpServer *server;
    QTcpSocket *clientSocket;
    QMap<QTcpSocket *, QString> connectedClients;
    bool serverMode;

    // --- Message framing helpers ---
    QMap<QTcpSocket *, QByteArray> buffers; // accumulate data per‑client (server mode)
    QByteArray clientBuffer;               // accumulate data when we are the client

    // Internal helpers
    void handleClientMessage(QTcpSocket *socket, const QJsonObject &message);
    QString generateClientId();
    void sendToClient(QTcpSocket *socket, const QJsonObject &message);
    void processBuffer(QByteArray &buffer, QTcpSocket *originSocket);
};

#endif // NETWORKMANAGER_H
