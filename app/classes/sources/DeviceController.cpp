#include "DeviceController.h"
#include <QNetworkInterface>
#include "PublicIpAddressFetcher.h"
#include "SessionUtilities.h"
#include <QJsonObject>
#include <QJsonDocument>

DeviceController::DeviceController(QObject *parent)
    : QObject{parent}
{
    connect(&_socket, &QTcpSocket::connected, this, &DeviceController::ConnectedToServer);
    connect(&_socket, &QTcpSocket::disconnected, this, &DeviceController::DisconnectedFromServer);
    connect(&_socket, &QTcpSocket::stateChanged, this, &DeviceController::SocketStateChanged);
    connect(&_socket, &QTcpSocket::errorOccurred, this, &DeviceController::ErrorOccurred);
    connect(&_socket, &QTcpSocket::readyRead, this, &DeviceController::ServerDataReady);
}

void DeviceController::ConnectToServer(QString Ip, int Port)
{
    if(_socket.isOpen())
    {
        if(Ip == _ip && Port == _port)
        {
            //DO nothing
            return;
        }
        _socket.close();
    }

    _ip = Ip;
    _port = Port;
    _socket.connectToHost(_ip, _port);
}

QAbstractSocket::SocketState DeviceController::GetState() const
{
    return _socket.state();
}

bool DeviceController::IsConnected() const
{
    return _socket.state() == QAbstractSocket::ConnectedState;
}

void DeviceController::SendDataToServer(QByteArray data)
{
    _socket.write(data);
}

QTcpSocket *DeviceController::GetServerSocket()
{
    return &_socket;
}

QTcpSocket *DeviceController::GetPlayerSocketById(int Id)
{
    return _socketsList.key(Id);
}

int DeviceController::GetPlayerIdBySocket(QTcpSocket *Socket, int DefaultVal)
{
    return _socketsList.value(Socket, DefaultVal);
}

void DeviceController::SetupTCPServer(int Port)
{
    if(!_server)
    {
        _server = new QTcpServer(this);
        connect(_server, &QTcpServer::newConnection, this, &DeviceController::ClientConnected);
    }
    bIsServerStarted = _server->listen(QHostAddress::Any, Port);
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
    if(_server && _server->isListening())
    {
        for(auto it = _socketsList.begin(); it != _socketsList.end(); ++it)
        {
            it.key()->close();
        }
        _server->close();
        bIsServerStarted = _server->isListening();
        _socketsList.clear();
        bIsServer = false;
        //
        emit ShutdownServerCall();
    }
}

void DeviceController::DisconnectFromServer()
{
    if(_socket.isOpen())
    {
        _socket.close();
    }
}

void DeviceController::SocketStateChanged(QAbstractSocket::SocketState state)
{
    if(state == QAbstractSocket::UnconnectedState)
    {
        _socket.close();
    }
    emit StateChanged(state);
}

void DeviceController::ServerDataReady()//Client received message from server.
{
    QByteArray data = _socket.readAll();
    emit ClientReceivedData(&_socket, data);
}

void DeviceController::ClientConnected()
{
    qDebug() << "a client connected to server";
    QTcpSocket* socket = _server->nextPendingConnection();
    _socketsList.insert(socket, _socketsList.size() + 1);//+1 because 0 is reserver by server

    connect(socket, &QTcpSocket::disconnected, this, &DeviceController::ClientDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &DeviceController::ClientDataReady);

    emit NewClientConnected(_socketsList[socket]);
}

void DeviceController::ClientDisconnected()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    int Id = -1;
    if(_socketsList.contains(socket))
    {
        Id = _socketsList.value(socket);
        _socketsList.remove(socket);
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
    if(_server)
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
        Info.append(" Port: " + QString::number(_server->serverPort()));
        return Info;
    }
    return QString("Server is not running");
}

void DeviceController::BroadcastToAllClients(QByteArray data, QTcpSocket* Except)
{
    for(auto it = _socketsList.begin(); it != _socketsList.end(); ++it)
    {
        if(Except != it.key())
        {
            it.key()->write(data);
        }
    }
}

// void DeviceController::SendDataToClient(QTcpSocket *Sender, QTcpSocket *Receiver, QByteArray data)
// {
//     //Do something with Sender. Maybe pass alongside with data? Or like inside struct.
//     Receiver->write(data);
// }

QList<QTcpSocket*> DeviceController::GetAllConnectedClients()
{
    return _socketsList.keys();
}
