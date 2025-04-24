#include "DeviceController.h"
#include <QNetworkInterface>
#include "PublicIpAddressFetcher.h"
#include "SessionUtilities.h"
#include <QJsonObject>
#include <QJsonDocument>

DeviceController::DeviceController(QObject *parent)
    : QObject{parent}
{
    connect(&Socket, &QTcpSocket::connected, this, &DeviceController::ConnectedToServer);
    connect(&Socket, &QTcpSocket::disconnected, this, &DeviceController::DisconnectedFromServer);
    connect(&Socket, &QTcpSocket::stateChanged, this, &DeviceController::SocketStateChanged);
    connect(&Socket, &QTcpSocket::errorOccurred, this, &DeviceController::ErrorOccurred);
    connect(&Socket, &QTcpSocket::readyRead, this, &DeviceController::ServerDataReady);
}

void DeviceController::ConnectToServer(QString inIp, int inPort)
{
    if(Socket.isOpen())
    {
        if(Ip == inIp && Port == inPort)
        {
            //DO nothing
            return;
        }
        Socket.close();
    }

    Ip = inIp;
    Port = inPort;
    Socket.connectToHost(Ip, Port);
}

QAbstractSocket::SocketState DeviceController::GetState() const
{
    return Socket.state();
}

bool DeviceController::IsConnected() const
{
    return Socket.state() == QAbstractSocket::ConnectedState;
}

void DeviceController::SendDataToServer(QByteArray data)
{
    Socket.write(data);
}

QTcpSocket *DeviceController::GetServerSocket()
{
    return &Socket;
}

QTcpSocket *DeviceController::GetPlayerSocketById(int Id)
{
    return SocketsList.key(Id);
}

int DeviceController::GetPlayerIdBySocket(QTcpSocket *Socket, int DefaultVal)
{
    return SocketsList.value(Socket, DefaultVal);
}

void DeviceController::SetupTCPServer(int Port)
{
    if(!Server)
    {
        Server = new QTcpServer(this);
        connect(Server, &QTcpServer::newConnection, this, &DeviceController::ClientConnected);
    }
    bIsServerStarted = Server->listen(QHostAddress::Any, Port);
    if(!bIsServerStarted)
    {
        qDebug() << "Server could not start";
    }
    else
    {
        qDebug() << "Server has started...";
        bIsServer = true;
        emit SetupServerCall();
    }
    ////
}

void DeviceController::ShutdownServer()
{
    if(Server && Server->isListening())
    {
        for(auto it = SocketsList.keyBegin(); it != SocketsList.keyEnd(); ++it)
        {
            QTcpSocket* Socket = it.base().key();
            Socket->close();
        }
        Server->close();

        bIsServerStarted = Server->isListening();
        SocketsList.clear();
        bIsServer = false;
        //
        emit ShutdownServerCall();
    }
}

void DeviceController::DisconnectFromServer()
{
    if(Socket.isOpen())
    {
        Socket.close();
    }
}

void DeviceController::SocketStateChanged(QAbstractSocket::SocketState state)
{
    if(state == QAbstractSocket::UnconnectedState)
    {
        Socket.close();
    }
    emit StateChanged(state);
}

void DeviceController::ServerDataReady()//Client received message from server.
{
    QByteArray data = Socket.readAll();
    emit ClientReceivedData(&Socket, data);
}

void DeviceController::ClientConnected()
{
    qDebug() << "a client connected to server";
    QTcpSocket* socket = Server->nextPendingConnection();
    SocketsList.insert(socket, SocketsList.size() + 1);//+1 because 0 is reserver by server

    connect(socket, &QTcpSocket::disconnected, this, &DeviceController::ClientDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &DeviceController::ClientDataReady);

    emit NewClientConnected(SocketsList[socket]);
}

void DeviceController::ClientDisconnected()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    int Id = -1;
    if(SocketsList.contains(socket))
    {
        Id = SocketsList.value(socket);
        SocketsList.remove(socket);
    }
    emit OldClientDisconnected(Id);
}

void DeviceController::ClientDataReady()//Server received message from clients
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());//Some client
    QByteArray data = socket->readAll();
    emit ServerReceivedData(socket, data);
}

bool DeviceController::IsServerStarted() const
{
    return bIsServerStarted;
}

bool DeviceController::IsServer() const
{
    return bIsServer;
}

QString DeviceController::GetServerRunInfo() const
{
    if(Server)
    {
        QString Info;
        // QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
        // for (int i = 0; i < ipAddressesList.size(); ++i) {
        //     if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
        //         ipAddressesList.at(i).toIPv4Address()) {
        //         break;
        //     }
        // }
        Info.append("IpAddress: " + fetcher.GetIpAddress());
        Info.append(" Port: " + QString::number(Server->serverPort()));
        return Info;
    }
    return QString("Server is not running");
}

void DeviceController::BroadcastToAllClients(QByteArray data, QTcpSocket* Except)
{
    for(auto it = SocketsList.begin(); it != SocketsList.end(); ++it)
    {
        if(Except != it.key())
        {
            it.key()->write(data);
        }
    }
}

QList<QTcpSocket*> DeviceController::GetAllConnectedClients()
{
    return SocketsList.keys();
}
