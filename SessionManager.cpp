#include "SessionManager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include "JsonFieldMediator.h"

SessionManager::SessionManager(QObject* parent)
    : QObject{parent}
{
    Controller = new DeviceController(this);
    connect(Controller, &DeviceController::NewClientConnected, this, &SessionManager::NewPlayerHandshake);
    connect(Controller, &DeviceController::OldClientDisconnected, this, &SessionManager::PlayerLeft);
    connect(Controller, &DeviceController::SetupServerCall, this, &SessionManager::SetupServerPlayer);
    connect(Controller, &DeviceController::ShutdownServerCall, this, &SessionManager::ClenupServerPlayer);
    connect(Controller, &DeviceController::ClientReceivedData, this, &SessionManager::ProcessServerData);
    connect(Controller, &DeviceController::ServerReceivedData, this, &SessionManager::ProcessClientData);
}


void SessionManager::StartGameSession()
{
    //To start game session we should gather data if all connected players are ready.
    //Make countdown of readiness for all players untill all are ready.
}

void SessionManager::InitPlayers()//Will be called when game start. Before calling we should have already received valid list of players in session from server.
{
    for(auto it = PlayerNameToUniqueId.begin(); it != PlayerNameToUniqueId.end(); ++it)
    {
        Players[it.key()] = PlayerEntry(it.value());
    }

    //LocalPlayerId = PlayerNameToUniqueId.key(PlayerName);
}

QByteArray SessionManager::GetFieldsToShareAndClear()
{
    return FieldsToShare.isEmpty() ? (QByteArray()) : (JSONFieldMediator::PackDataAsMappingInplace(FieldsToShare));
}

void SessionManager::InitLocalPlayer(QString NewName)
{
    if(PlayerName != NewName)
    {
        PlayerName = NewName;
    }
    LocalPlayerId = 0;
    PlayerNameToUniqueId[LocalPlayerId] = PlayerName;
}

void SessionManager::SetFieldsToShare(const QMap<QString, QString>& NewFields)
{
    if(!FieldsToShare.isEmpty())
    {
        FieldsToShare.clear();
    }
    FieldsToShare.insert(NewFields);
}

void SessionManager::UpdatePlayerList(const QByteArray &Data)
{
    //Not optimal - clearing whole map when updating.TODO: mesure before optimizing
    PlayerNameToUniqueId.clear();
    QJsonDocument Doc = QJsonDocument::fromJson(Data);
    if(!Doc.isNull())
    {
        if(Doc.isObject())
        {
            QJsonObject JsonObj = Doc.object();
            for(auto it = JsonObj.begin(); it != JsonObj.end(); ++it)
            {
                if(it.value().isString())
                {
                    PlayerNameToUniqueId.insert(it.key().toInt(), it.value().toString("Null"));
                }
            }
        }
    }
}

void SessionManager::SendMessage(MessageFlags Flag, QTcpSocket *Socket/*, const QByteArray &Payload*/)
{
    //TODO make message based on given flag. Payload could be ignored or even discarded from function.
    QJsonDocument Doc;
    QJsonObject Object;
    QByteArray ResponseData;

    //Maybe I could use bIsServer variable to make it clear how and what message we should send
    switch(Flag)// Which message we want to create and send to given socket.
    {
    case MessageFlags::BroadcastFieldsReviel:
        //Message signature 3....A...........{"FieldName":"FieldValue",etc...}
        //TODO check if works correctly
        ResponseData.append(static_cast<char>(MessageFlags::BroadcastFieldsReviel));
        ResponseData.append(static_cast<char>(LocalPlayerId));//WHOISSENDER Pass into message as a char. Actually could be a huge problem. If char tables are not unified for devices.
        ResponseData.append(GetFieldsToShareAndClear());

        if(Controller->IsServer())
        {
            //We are server as client - we should just broadcast to all connected devices fields.
            Controller->BroadcastToAllClients(ResponseData);
        }
        else
        {
            //Client made message for broadcast. We should inform server about call and let it handle broadcasting.
            Controller->SendDataToServer(ResponseData);
        }

        qDebug() << "Client sent message to server for broadcasting fields to all players";
        break;

    case MessageFlags::InitPlayerList:
        if(Controller->IsServer())// If we are not the server we have no reason to create message with this flag
        {
            ResponseData.append(static_cast<char>(MessageFlags::InitPlayerList));
            for(auto it = PlayerNameToUniqueId.begin(); it != PlayerNameToUniqueId.end(); ++it)
            {
                Object.insert(QString::number(it.key()), it.value());
            }
            Doc.setObject(Object);
            ResponseData.append(Doc.toJson(QJsonDocument::Compact));

            Controller->BroadcastToAllClients((ResponseData));
        }
        break;

    case MessageFlags::ReqInitPlayer:
        //socket->write("Welcome to the server.");
        if(Controller->IsServer())
        {
            ResponseData.append(static_cast<char>(MessageFlags::ReqInitPlayer));
            if(!Socket)
            {
                qDebug() << "Trying to use nullptr socket when ReqInitPlayer message.\n";
            }
            else
            {
                Socket->write(ResponseData);
            }
        }
        break;

    case MessageFlags::ResInitPlayer:
        if(!Controller->IsServer())//We only required to make this message as clients. On server we instantly can initialize player as server was initialized.
        {
            Object.insert("Nickname", PlayerName);
            Doc.setObject(Object);
            ResponseData.append(static_cast<char>(MessageFlags::ResInitPlayer));
            ResponseData.append(Doc.toJson(QJsonDocument::Compact));
            if(!Socket)
            {
                qDebug() << "Trying to use nullptr socket when ResInitPlayer message.\n";
            }
            else
            {
                Socket->write(ResponseData);
            }
        }
        break;

    case MessageFlags::TargetFieldsReviel:
        //We should create message to given socket with field that we want to reviel to someone specific.
        //TODO
        break;

    default:
        break;
    }
}

void SessionManager::ProcessServerData(QTcpSocket* Sender, QByteArray Data)//Client received data from server.
{
    MessageFlags ReadFlag = static_cast<MessageFlags>(Data.at(0));
    QJsonDocument Doc;
    QMap<QString, QString> ReceivedFields;

    int PlayerIndex = -1;

    switch(ReadFlag)//Client received message with this flag
    {
    case MessageFlags::ReqInitPlayer:

        //Check if it works
        SendMessage(MessageFlags::ResInitPlayer, Sender);
        break;

    case MessageFlags::TargetFieldsReviel:
        //SendMessage(socket, ReadFlag, data);
        break;

    case MessageFlags::BroadcastFieldsReviel:

        //TODO Check if works

        //Move functionality to session manager
        PlayerIndex = static_cast<int>(Data.at(1));
        Doc = QJsonDocument::fromJson(Data.right(Data.size() - 2));
        //Save data
        ReceivedFields = JSONFieldMediator::UnpackDataAsMappingInplace(Doc);

        Players[PlayerIndex].AddUpdateEntryBulk(ReceivedFields);


        emit ReceivedBroadcastedPlayerFields(PlayerIndex, ReceivedFields);//TODO: connect to the signal
        break;

    case MessageFlags::InitPlayerList:
        UpdatePlayerList(Data);
        break;

    default:
        qDebug() << "Not handled message from server!\n";
        break;
    }
}

void SessionManager::ProcessNewPlayerName(QTcpSocket* Sender, QByteArray Data)
{
    QJsonDocument Doc = QJsonDocument::fromJson(Data);
    QJsonObject Object;
    QString ReceivedNickname;
    if(!Doc.isNull())
    {
        if(Doc.isObject())
        {
            Object = Doc.object();
            ReceivedNickname = Object["Nickname"].toString("null");

            int UniquePlayerId = Controller->GetPlayerIdBySocket(Sender, -1);

            if(UniquePlayerId == -1)//First entry. Somehow we received message from unkown socket. Just drop.
            {
                return;
            }

            if(!PlayerNameToUniqueId.contains(UniquePlayerId))//First entry. Add unique Id for player and write player nickname
            {
                if(ReceivedNickname == "null")
                {
                    //Handle invalid nickname.
                }
                PlayerNameToUniqueId.insert(UniquePlayerId, ReceivedNickname);
            }
            else
            {
                //Update nickname
                QString OldName = PlayerNameToUniqueId.value(UniquePlayerId);
                if(OldName == ReceivedNickname)
                {
                    //Same nickname. Do nothing.
                    return;
                }
                else
                {
                    PlayerNameToUniqueId.remove(UniquePlayerId);
                    PlayerNameToUniqueId.insert(UniquePlayerId, ReceivedNickname);
                }
            }
        }
    }
}

void SessionManager::ProcessClientData(QTcpSocket* Sender, QByteArray Data)//Server received data from client.
{
    MessageFlags ReadFlag = static_cast<MessageFlags>(Data.at(0));
    QJsonDocument Doc;
    QJsonObject Object;
    QString ReceivedNickname;

    int PlayerIndex = -1;

    switch(ReadFlag)//Server received message and this is flag of this message
    {
    case MessageFlags::ResInitPlayer:

        //I need to extract player nickname here before broadcasting.
        ProcessNewPlayerName(Sender, Data.right(Data.size() - 1));
        SendMessage(MessageFlags::InitPlayerList);//TODO Check if this broadcast init works
        break;
        //                                             Brdcst flag  Sender(char)    JSON Payload
        //                                                      |    |                              |
    case MessageFlags::BroadcastFieldsReviel://Message signature 3....A...........{"FieldName":"FieldValue",etc...}
        //TODO check if works correctly
        PlayerIndex = static_cast<int>(Data.at(1));
        Doc = QJsonDocument::fromJson(Data.right(Data.size() - 2));


        //Actually we can just broadcast this data to all clients and unpack it on server (in case of server as a client).
        Controller->BroadcastToAllClients(Data, Controller->GetPlayerSocketById((PlayerIndex)));
        //Save data
        Players[PlayerIndex].AddUpdateEntryBulk(JSONFieldMediator::UnpackDataAsMappingInplace(Doc));
        //TODO: I need to create signals in player entity to be able to show changes of broadcasted fields.
        qDebug() << "Server should broadcast passed info to all players";
        break;

    case MessageFlags::TargetFieldsReviel:
        //TODO. Later
        qDebug() << "Server got message for player target field reviel.";
        break;

    case MessageFlags::ReqInitPlayer:
        qDebug() << "Server got request for init player. It is prohibited.";
        break;
    default:
        qDebug() << "Not handled message from server!\n";
        break;
    }
}

void SessionManager::SetupServerPlayer()
{
    InitLocalPlayer(PlayerName);
}

void SessionManager::ClenupServerPlayer()
{
    PlayerNameToUniqueId.removeIf([this](QPair<int, QString> KeyValue)
                                  {
                                      return KeyValue.second == PlayerName;
                                  });
}

void SessionManager::NewPlayerHandshake(int Id)
{
    //Send handshake message to a client and ask for their nickname
    SendMessage(MessageFlags::ReqInitPlayer, Controller->GetPlayerSocketById(Id));
}

void SessionManager::PlayerLeft(int Id)
{
    //TODO: Do something when player left
}
