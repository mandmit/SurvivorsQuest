#ifndef DEVICECONTROLLER_H
#define DEVICECONTROLLER_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include "PublicIpAddressFetcher.h"

class DeviceController : public QObject
{
    Q_OBJECT
public:
    explicit DeviceController(QObject *parent = nullptr);


    //Client methods
    void ConnectToServer(QString Ip, int Port);
    void DisconnectFromServer();
    QAbstractSocket::SocketState GetState() const;
    bool IsConnected() const;
    void SendDataToServer(QByteArray Data);
    QTcpSocket* GetServerSocket();

    //Shared methods
    //void SendMessage(MessageFlags Flag, QTcpSocket* Socket = nullptr, const QByteArray& Payload = QByteArray());

    //Server methods
    QTcpSocket* GetPlayerSocketById(int Id);
    int GetPlayerIdBySocket(QTcpSocket* Socket, int DefaultVal = -1);
    void SetupTCPServer(int Port);
    void ShutdownServer();
    bool IsServerStarted() const;
    bool IsServer() const;
    QString GetServerRunInfo() const;
    void BroadcastToAllClients(QByteArray Data, QTcpSocket* Except = nullptr);
    //void SendDataToClient(QTcpSocket* Sender, QTcpSocket* Receiver, QByteArray Data);
    QList<QTcpSocket*> GetAllConnectedClients();

public:
    PublicIpAddressFetcher fetcher;

signals:
    //Client signals
    void ConnectedToServer();
    void DisconnectedFromServer();
    void StateChanged(QAbstractSocket::SocketState);
    void ErrorOccurred(QAbstractSocket::SocketError);
    void ClientReceivedData(QTcpSocket* Sender, QByteArray Data);

    //Shared signals
    void ReceivedBroadcastedPlayerFields(QByteArray Data);
    void ReceivedInitPlayerList(QByteArray Data);

    //Server signals
    void SetupServerCall();
    void NewClientConnected(int Id);
    void OldClientDisconnected(int Id);
    void ShutdownServerCall();
    void ServerReceivedData(QTcpSocket* Sender, QByteArray Data);


private slots:
    //Client slots
    void SocketStateChanged(QAbstractSocket::SocketState);
    void ServerDataReady();

    //Server slots
    void ClientConnected();
    void ClientDisconnected();
    void ClientDataReady();
private:
    int LocalUniqueId = -1;
    //Client stuf

    QTcpSocket _socket;//Client socket - connection to the server
    QString _ip;

    bool bIsServer = false;
    int _port;

    //Server stuf
    QTcpServer* _server = nullptr;
    QMap<QTcpSocket*, int> _socketsList;
    bool bIsServerStarted = false;
};

#endif // DEVICECONTROLLER_H
