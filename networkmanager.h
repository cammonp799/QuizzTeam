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
    
    // Server methods (for host)
    bool startServer(quint16 port = 0);
    void stopServer();
    quint16 getServerPort() const;
    
    // Client methods (for players)
    void connectToHost(const QString& hostAddress, quint16 port);
    void disconnectFromHost();
    
    // Message sending
    void sendMessage(const QJsonObject& message);
    void broadcastMessage(const QJsonObject& message);
    
    // Connection status
    bool isServer() const;
    bool isConnected() const;
    QStringList getConnectedClients() const;

private slots:
    void onNewConnection();
    void onClientDisconnected();
    void onDataReceived();

signals:
    void serverStarted(quint16 port);
    void serverStopped();
    void clientConnected(const QString& clientId);
    void clientDisconnected(const QString& clientId);
    void connectedToHost();
    void disconnectedFromHost();
    void messageReceived(const QJsonObject& message, const QString& senderId);
    void connectionError(const QString& error);

private:
    QTcpServer* server;
    QTcpSocket* clientSocket;
    QMap<QTcpSocket*, QString> connectedClients;
    bool serverMode;
    
    void handleClientMessage(QTcpSocket* socket, const QJsonObject& message);
    QString generateClientId();
    void sendToClient(QTcpSocket* socket, const QJsonObject& message);
};

#endif // NETWORKMANAGER_H