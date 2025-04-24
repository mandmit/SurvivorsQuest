#include "SessionManager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
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

    FieldsForGame = DefaultFieldToValuesMapping.keys();
}


void SessionManager::PreStartGameSession()
{
    //To start game session we should gather data if all connected players are ready.
    //Make countdown of readiness for all players untill all are ready.
    //For now we should only make network call for start game (each client should enter screen with all players fields, server as well)
    InitPlayers();

    if(Controller->IsServer())
    {
        SendMessage(MessageFlags::StartGame);

    }
    emit StartLocalSession();
}

void SessionManager::InitPlayers()//Will be called when game starts. Before calling we should have already received valid list of players in session from server.
{
    bool bIsPlayerEntryOwner = false;
    bool bIsAuthority = Controller->IsServer();
    for(auto it = PlayerNameToUniqueId.begin(); it != PlayerNameToUniqueId.end(); ++it)
    {
        PlayerEntry Entry{it.value()};
        bIsPlayerEntryOwner = (PlayerName == it.value());

        if(bIsAuthority)
        {
            for(const QString& FieldName : FieldsForGame)
            {
                //TODO Rework structure representation of mapping field name to value later - using QVariant as value instead of plain qstring.
                if(FieldName == "Age")
                {
                    Entry.AddUpdateEntry({"Age", QString::number(QRandomGenerator::global()->bounded(18, 90))}, bIsPlayerEntryOwner);
                }
                else
                {
                    const size_t FieldValuesSize = DefaultFieldToValuesMapping[FieldName].size();
                    Entry.AddUpdateEntry({FieldName, DefaultFieldToValuesMapping[FieldName].at(QRandomGenerator::global()->bounded(FieldValuesSize))}, bIsPlayerEntryOwner);
                }
            }
        }
        else
        {
            Entry.InitPlayerEntry(FieldsForGame, "???");
        }
        //Players[it.key()] = std::move(Entry);
        Players.insert(it.key(), std::move(Entry));
    }


    if(bIsAuthority)
    {
        SendMessage(MessageFlags::InitPlayerFields);//Send to all client sockets their initial player fields.
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
                    QString Name = it.value().toString("Null");
                    PlayerNameToUniqueId.insert(it.key().toInt(), Name);
                    if(Name == PlayerName)
                    {
                        LocalPlayerId = it.key().toInt();
                    }
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
    QByteArray Message;

    //Maybe I could use bIsServer variable to make it clear how and what message we should send
    switch(Flag)// Which message we want to create and send to given socket.
    {
    case MessageFlags::BroadcastFieldsReveal:
        //Message signature 3....A...........{"FieldName":"FieldValue",etc...}
        //TODO check if works correctly
        Message.append(static_cast<char>(MessageFlags::BroadcastFieldsReveal));
        Message.append(static_cast<char>(LocalPlayerId));//WHOISSENDER Pass into message as a char. Actually could be a huge problem. If char tables are not unified for devices.
        Message.append(GetFieldsToShareAndClear());

        if(Controller->IsServer())
        {
            //We are server as client - we should just broadcast to all connected devices fields.
            Controller->BroadcastToAllClients(Message);
        }
        else
        {
            //Client made message for broadcast. We should inform server about call and let it handle broadcasting.
            Controller->SendDataToServer(Message);
        }

        qDebug() << "Client sent message to server for broadcasting fields to all players";
        break;

    case MessageFlags::InitPlayerList:
        if(Controller->IsServer())// If we are not the server we have no reason to create message with this flag
        {
            Message.append(static_cast<char>(MessageFlags::InitPlayerList));
            for(auto it = PlayerNameToUniqueId.begin(); it != PlayerNameToUniqueId.end(); ++it)
            {
                Object.insert(QString::number(it.key()), it.value());
            }
            Doc.setObject(Object);
            Message.append(Doc.toJson(QJsonDocument::Compact));

            Controller->BroadcastToAllClients((Message));
        }
        break;

    case MessageFlags::ReqInitPlayer:
        //socket->write("Welcome to the server.");
        if(Controller->IsServer())
        {
            Message.append(static_cast<char>(MessageFlags::ReqInitPlayer));
            if(!Socket)
            {
                qDebug() << "Trying to use nullptr socket when ReqInitPlayer message.\n";
            }
            else
            {
                Socket->write(Message);
            }
        }
        break;

    case MessageFlags::ResInitPlayer:
        if(!Controller->IsServer())//We only required to make this message as clients. On server we instantly can initialize player as server was initialized.
        {
            Object.insert("Nickname", PlayerName);
            Doc.setObject(Object);
            Message.append(static_cast<char>(MessageFlags::ResInitPlayer));
            Message.append(Doc.toJson(QJsonDocument::Compact));
            if(!Socket)
            {
                qDebug() << "Trying to use nullptr socket when ResInitPlayer message.\n";
            }
            else
            {
                Socket->write(Message);
            }
        }
        break;

    case MessageFlags::TargetFieldsReveal:
        //We should create message to given socket with field that we want to reveal to someone specific.
        //TODO
        break;

    case MessageFlags::InitPlayerFields:
        if(!Controller.isNull() && Controller->IsServer())
        {
            for(QTcpSocket* ClientSocket : Controller->GetAllConnectedClients())
            {
                QMap<QString, QString> PlayerFieldsCopy = Players[Controller->GetPlayerIdBySocket(ClientSocket)].GetPlayerFieldsCopyAsStrings();

                Message.append(static_cast<char>(MessageFlags::InitPlayerFields));
                Message.append(JSONFieldMediator::PackDataAsMappingInplace(PlayerFieldsCopy));

                ClientSocket->write(Message);
            }
        }

        break;

    case MessageFlags::ReqChangeNicknameDublicate:
        Message.append(static_cast<char>(MessageFlags::ReqChangeNicknameDublicate));
        if(!Socket)
        {
            qDebug() << "Invalid socket pointer\n";
        }
        else
        {
            Socket->write(Message);
        }
        break;

    case MessageFlags::StartGame:
        if(Controller->IsServer())
        {
            Message.append(static_cast<char>(MessageFlags::StartGame));
            Controller->BroadcastToAllClients(Message);
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

QVector<PlayerEntry *> SessionManager::GetPlayerEntriesList()
{
    QVector<PlayerEntry*> EntriesList;
    EntriesList.reserve(Players.size());

    for(auto It = Players.begin(); It != Players.end(); ++It)
    {
        if(It.value().GetPlayerName() != PlayerName)//Add if not the owner of fields.
        {
            EntriesList.push_back(&It.value());
        }
        else
        {
            LocalPlayer = &It.value();
        }
    }

    return EntriesList;
}

QList<QString> SessionManager::GetPlayersListNames() const
{
    return PlayerNameToUniqueId.values();
}

DeviceController *SessionManager::GetController()
{
    return Controller.get();
}

PlayerEntry *SessionManager::GetLocalPlayerEntry()
{
    if(!LocalPlayer)
    {
        LocalPlayer = &Players[LocalPlayerId];
    }
    return LocalPlayer;
}

QStringListModel *SessionManager::GetPlayerListViewModel()
{
    return Model.get();
}

void SessionManager::SetFieldsForSession(const QList<QString> &Fields, const QString& DefaultValue)
{
    if(!Fields.isEmpty())
    {
        FieldsForGame.clear();
    }
    FieldsForGame = std::move(Fields);
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

    case MessageFlags::TargetFieldsReveal:
        //SendMessage(socket, ReadFlag, data);
        break;

    case MessageFlags::BroadcastFieldsReveal:

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

    case MessageFlags::InitPlayerFields:

        Doc = QJsonDocument::fromJson(Data.right(Data.size() - 1));
        ReceivedFields = JSONFieldMediator::UnpackDataAsMappingInplace(Doc);

        Players[LocalPlayerId].InitPlayerEntry(ReceivedFields);

        //emit ClientReceivedInitialPlayerFields();//Does this call even make sense? We updated our player representation data already.

        break;

    case MessageFlags::StartGame:

        PreStartGameSession();
        break;

    default:
        qDebug() << "Not handled message from server!\n";
        break;
    }
}

void SessionManager::ProcessNewClientPlayerName(QTcpSocket* Sender, const QByteArray& Data)//Called only on server
{
    if(!Controller.isNull() && Controller->IsServer())
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
                            //TODO: Problem if client sent his identical name as before that was claimed as dublicate - it will be ignored and not be processed.
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
}

void SessionManager::UpdatePlayerListView()
{
    PlayerListView.clear();
    PlayerListView = GetPlayersListNames();

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
    case MessageFlags::BroadcastFieldsReveal://Message signature 3....A...........{"FieldName":"FieldValue",etc...}
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

    case MessageFlags::TargetFieldsReveal:
        //TODO. Later
        qDebug() << "Server got message for player target field reveal.";
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
