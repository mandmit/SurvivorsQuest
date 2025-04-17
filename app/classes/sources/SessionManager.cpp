#include "SessionManager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include "JsonFieldMediator.h"

SessionManager::SessionManager(QObject* parent)
    : QObject{parent}
{
    Controller.reset(new DeviceController(this));
    connect(Controller.get(), &DeviceController::NewClientConnected, this, &SessionManager::NewPlayerHandshake);
    connect(Controller.get(), &DeviceController::OldClientDisconnected, this, &SessionManager::PlayerLeft);
    connect(Controller.get(), &DeviceController::SetupServerCall, this, &SessionManager::SetupServerPlayer);
    connect(Controller.get(), &DeviceController::ShutdownServerCall, this, &SessionManager::ClenupServerPlayer);
    connect(Controller.get(), &DeviceController::ClientReceivedData, this, &SessionManager::ProcessServerData);
    connect(Controller.get(), &DeviceController::ServerReceivedData, this, &SessionManager::ProcessClientData);

    connect(Controller.get(), &DeviceController::ConnectedToServer, this, &SessionManager::ProcessServerConnection);
    connect(Controller.get(), &DeviceController::DisconnectedFromServer, this, &SessionManager::ProcessServerDisconnection);

    Model.reset(new QStringListModel(this));
    Model->setStringList(PlayerListView);
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

void SessionManager::ChangePlayerName(const QString& NewName)
{
    if(PlayerName != NewName)
    {
        PlayerName = NewName;
        if(!Controller->IsServer())
        {
            ServerNotifyClientNameUpdated();
        }

        //emit ChangeNicknameStatus(true);
    }
    else
    {
        //emit ChangeNicknameStatus(false);
    }
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
    UpdatePlayerListView();
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

    case MessageFlags::ReqChangeNicknameDublicate:
        //TODO send message to change nickname.
        ResponseData.append(static_cast<char>(MessageFlags::ReqChangeNicknameDublicate));
        if(!Socket)
        {
            qDebug() << "Invalid socket pointer\n";
        }
        else
        {
            Socket->write(ResponseData);
        }
        break;

    default:
        break;
    }
}

QString SessionManager::GetPlayerNicknameById(int Id)
{
    return (PlayerNameToUniqueId.value(Id, "null"));
}

QList<QString> SessionManager::GetPlayersList() const
{
    return PlayerNameToUniqueId.values();
}

DeviceController *SessionManager::GetController()
{
    return Controller.get();
}

QStringListModel *SessionManager::GetPlayerListViewModel()
{
    return Model.get();
}

void SessionManager::ProcessServerData(QTcpSocket* Sender, const QByteArray& Data)//Client received data from server.
{
    MessageFlags ReadFlag = static_cast<MessageFlags>(Data.at(0));
    QJsonDocument Doc;
    QMap<QString, QString> ReceivedFields;

    int PlayerIndex = -1;

    switch(ReadFlag)//Client received message with this flag
    {
    case MessageFlags::ReqInitPlayer:
        SendMessage(MessageFlags::ResInitPlayer, Sender);
        break;

    case MessageFlags::TargetFieldsReviel:
        //SendMessage(socket, ReadFlag, data);
        break;

    case MessageFlags::BroadcastFieldsReviel:

        //TODO Check if works

        PlayerIndex = static_cast<int>(Data.at(1));
        Doc = QJsonDocument::fromJson(Data.right(Data.size() - 2));
        //Save data
        ReceivedFields = JSONFieldMediator::UnpackDataAsMappingInplace(Doc);

        Players[PlayerIndex].AddUpdateEntryBulk(ReceivedFields);


        emit ReceivedBroadcastedPlayerFields(PlayerIndex, ReceivedFields);//TODO: connect to the signal
        break;

    case MessageFlags::InitPlayerList:
        UpdatePlayerList(Data.right(Data.size() - 1));
        break;

    case MessageFlags::ReqChangeNicknameDublicate:
        emit RequestChangePlayerNickname();
        break;

    default:
        qDebug() << "Not handled message from server!\n";
        break;
    }
}

void SessionManager::ProcessNewClientPlayerName(QTcpSocket* Sender, const QByteArray& Data)
{
    QJsonParseError Error;
    QJsonDocument Doc = QJsonDocument::fromJson(Data, &Error);
    QJsonObject Object;
    QString ReceivedNickname;

    //Data = Data.trimmed();

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
                if(ReceivedNickname == "null" || ReceivedNickname.isEmpty())
                {
                    //Handle invalid nickname.
                    qDebug() << "Received Invalid nickname";
                }
                else
                {
                    //Check if the current nickname has not already been taken
                    if(PlayerNameToUniqueId.key(ReceivedNickname, -1) != -1)
                    {
                        //TODO: Problem if client sent his identical name as before that was claimed as dublicate.
                        //Make message for this client to change nickname.
                        SendMessage(MessageFlags::ReqChangeNicknameDublicate, Sender);
                        return;
                    }
                    PlayerNameToUniqueId.insert(UniquePlayerId, ReceivedNickname);
                }
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

            UpdatePlayerListView();
            emit NewPlayerNicknameReceived(UniquePlayerId, ReceivedNickname);
        }
    }
    else
    {
        if (Error.error != QJsonParseError::NoError)
            qDebug() << "Parsing failed:" << Error.errorString();
    }
}

void SessionManager::UpdatePlayerListView()
{
    PlayerListView.clear();
    PlayerListView = GetPlayersList();

    Model->setStringList(PlayerListView);
}

void SessionManager::ProcessClientData(QTcpSocket* Sender, const QByteArray& Data)//Server received data from client.
{
    MessageFlags ReadFlag = static_cast<MessageFlags>(Data.at(0));
    QJsonDocument Doc;

    int PlayerIndex = -1;

    switch(ReadFlag)//Server received message and this is flag of this message
    {
    case MessageFlags::ResInitPlayer:

        //I need to extract player nickname here before broadcasting.
        ProcessNewClientPlayerName(Sender, Data.right(Data.size() - 1));
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
        qDebug() << "Not handled message from client!\n";
        break;
    }
}

void SessionManager::SetupServerPlayer()
{
    ChangePlayerName(PlayerName);

    LocalPlayerId = 0;
    PlayerNameToUniqueId[LocalPlayerId] = PlayerName;

    UpdatePlayerListView();
}

void SessionManager::ClenupServerPlayer()
{
    PlayerNameToUniqueId.clear();
    UpdatePlayerListView();//Call to update view model.

}

void SessionManager::NewPlayerHandshake(int Id)
{
    //Send handshake message to a client and ask for their nickname
    SendMessage(MessageFlags::ReqInitPlayer, Controller->GetPlayerSocketById(Id));
}

void SessionManager::PlayerLeft(int Id)
{
    QString LeftPlayerName = GetPlayerNicknameById(Id);
    PlayerNameToUniqueId.remove(Id);

    UpdatePlayerListView();

    emit PlayerWithNicknameLeft(Id, LeftPlayerName);
}

void SessionManager::ProcessServerConnection()
{
    PlayerNameToUniqueId.clear();
}

void SessionManager::ProcessServerDisconnection()
{
    PlayerNameToUniqueId.clear();
}

void SessionManager::ServerNotifyClientNameUpdated()
{
    SendMessage(MessageFlags::ResInitPlayer, Controller->GetServerSocket());
}
