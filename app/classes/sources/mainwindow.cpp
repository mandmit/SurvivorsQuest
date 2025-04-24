#include "mainwindow.h"
#include "PromptDialog.h"
#include "ui_mainwindow.h"

#include <QHostAddress>
#include <QMetaEnum>
#include <QStyle>

#include "SessionUtilities.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    Manager.reset(new SessionManager(this));
    SetDeviceControllerCallbacks();
    SetSessionManagerCallbacks(true);


    if (!PInfoWidget) { // Ensure only one instance exists
        //PInfoWidget = new PlayerInfoWindow();
        //connect(PInfoWidget, &PlayerInfoWindow::UpdateButtonClicked, this, &MainWindow::SendPlayerInfoToTheServer);


        //Tempo Only test

        // PlayerEntry PlayerEntry;
        // PlayerEntry.InitPlayerEntry({"Age", "Name", "Ocupation"}, "Just tempon variable");
        // PInfoWidget->InitPlayer(PlayerEntry);
    }
    ui->mainWindowStackedWidget->setCurrentIndex(MainMenuWidgetIndexes::Home);
    ui->horizontalFrameHeader->setVisible(false);

}

MainWindow::~MainWindow()
{
    delete ui;
    if (PInfoWidget) {
        delete PInfoWidget;
    }
}

void MainWindow::SetupLocalPlayerName(const QString& PlayerName)
{
    Manager->ChangePlayerName(PlayerName);
}

void MainWindow::SetDeviceControllerCallbacks()
{
    connect(Manager->GetController(), &DeviceController::ConnectedToServer, this, &MainWindow::ConnectedToServer);
    connect(Manager->GetController(), &DeviceController::DisconnectedFromServer, this, &MainWindow::DisconnectedFromServer);
    connect(Manager->GetController(), &DeviceController::StateChanged, this, &MainWindow::DeviceStateChanged);
    connect(Manager->GetController(), &DeviceController::ErrorOccurred, this, &MainWindow::DeviceErrorOccurred);
    connect(Manager->GetController(), &DeviceController::ClientReceivedData, this, &MainWindow::DeviceDataReady);

    connect(Manager.get(), &SessionManager::StartLocalSession, this, &MainWindow::OnSessionStartReady);


    connect(&Manager->GetController()->fetcher, &PublicIpAddressFetcher::IpAddressReceived, this, &MainWindow::ServerUpdatePublicIP);
}

void MainWindow::SetSessionManagerCallbacks(bool bBindUnbind)
{
    if(bBindUnbind)
    {
        connect(Manager.get(), &SessionManager::NewPlayerNicknameReceived, this, &MainWindow::NewClientConnected);
        connect(Manager.get(), &SessionManager::PlayerWithNicknameLeft, this, &MainWindow::ClientDisconnected);
        //connect(Manager.get(), &SessionManager::ChangeNicknameStatus, this, &MainWindow::CheckClientValidNickname);
        connect(Manager.get(), &SessionManager::RequestChangePlayerNickname, this, &MainWindow::TriggerChangeNicknameWindow);
    }
    else
    {
        disconnect(Manager.get(), &SessionManager::NewPlayerNicknameReceived, this, &MainWindow::NewClientConnected);
        disconnect(Manager.get(), &SessionManager::PlayerWithNicknameLeft, this, &MainWindow::ClientDisconnected);
        //disconnect(Manager.get(), &SessionManager::ChangeNicknameStatus, this, &MainWindow::CheckClientValidNickname);
        disconnect(Manager.get(), &SessionManager::RequestChangePlayerNickname, this, &MainWindow::TriggerChangeNicknameWindow);
    }
}

void MainWindow::InitPlayerOnTabWidget()
{

    PlayerInfoWidget* EntryWidget = nullptr;

    const QVector<PlayerEntry*> Players = Manager->GetPlayerEntriesList();

    for(const PlayerEntry* Entry : Players)
    {
        EntryWidget = new PlayerInfoWidget(this);//QWidget
        //connect(PInfoWidget, &PlayerInfoWidget::UpdateButtonClicked, this, &MainWindow::SendPlayerInfoToTheServer);
        EntryWidget->InitPlayer(*Entry);

        ui->playersTabWidget->addTab(EntryWidget, Entry->GetPlayerName());
    }

    const PlayerEntry* LocalPlayer = Manager->GetLocalPlayerEntry();

    EntryWidget = new PlayerInfoWidget(ui->localPlayerFrame);//QWidget
    EntryWidget->InitPlayer(*LocalPlayer);
}

void MainWindow::on_ClientIpAddress_textChanged(const QString &arg1)
{
    QHostAddress Address(arg1);
    QString State = "0";
    if(arg1 == "...")
    {
        State = "";
    }
    else
    {
        if(QAbstractSocket::IPv4Protocol == Address.protocol())
        {
            State = "1";
        }
    }
    ui->ClientIndicator->setProperty("state", State);
    style()->polish(ui->ClientIndicator);
}


void MainWindow::on_clientConnectBtn_clicked()
{

    if(Manager->GetController()->IsServerStarted())
    {
        qDebug() << "Currently your session is in server mode. Shutdown server to be able to connect as an client.";
        return;
    }
    if(Manager->GetController()->IsConnected())
    {
        Manager->GetController()->DisconnectFromServer();
    }
    else
    {
        auto Ip = ui->ClientIpAddress->text();
        auto Port = ui->ClientPort->value();
        Manager->GetController()->ConnectToServer(Ip, Port);
    }
}

void MainWindow::ConnectedToServer()
{
    ui->connectedPlayersListView_Client->setModel(Manager->GetPlayerListViewModel());
    ui->LstConsole->addItem("Connected to Server");
    ui->clientConnectBtn->setText("Disconnect");
}

void MainWindow::DisconnectedFromServer()
{
    ui->connectedPlayersListView_Client->reset();//Does not work
    ui->LstConsole->addItem("Disconnected from Server");
    ui->clientConnectBtn->setText("Connect");
}

void MainWindow::DeviceStateChanged(QAbstractSocket::SocketState state)
{
    QMetaEnum MetaEnum = QMetaEnum::fromType<QAbstractSocket::SocketState>();
    ui->LstConsole->addItem(MetaEnum.valueToKey(state));
}

void MainWindow::DeviceErrorOccurred(QAbstractSocket::SocketError Error)
{
    QMetaEnum MetaEnum = QMetaEnum::fromType<QAbstractSocket::SocketError>();
    ui->LstConsole->addItem(MetaEnum.valueToKey(Error));
}

void MainWindow::DeviceDataReady(QTcpSocket* Sender, const QByteArray& Data)
{
    ui->LstConsole->addItem(QString(Data));
}

void MainWindow::TriggerChangeNicknameWindow()
{
    PromptDialog prompt;
    if (prompt.exec() == QDialog::Accepted) {
        QString Name = prompt.GetText();
        if(!Name.isNull() && !Name.isEmpty())
        {
            Manager->ChangePlayerName(Name);
        }
        else
        {
            qDebug() << "Client entered non valid name. It is empty or null.\n";
        }
    }
}

void MainWindow::OnSessionStartReady()
{
    ui->mainWindowStackedWidget->setCurrentIndex(MainMenuWidgetIndexes::InGame);
    ui->horizontalFrameHeader->setVisible(true);

    InitPlayerOnTabWidget();
}


void MainWindow::ClientDataReceived(QTcpSocket* Sender, const QByteArray& ReceivedData)//Data received by current device
{
    ui->LstConsole->addItem(QString(ReceivedData));
    MessageFlags Flag = static_cast<MessageFlags>(ReceivedData.at(0));
    switch(Flag)
    {
    case MessageFlags::BroadcastFieldsReveal:
        ui->LstConsole->addItem(QString("BroadcastFieldReveal message."));
        PInfoWidget->WriteFieldsDataFromMapping(JSONFieldMediator::UnpackDataAsMappingInplace(ReceivedData.right(ReceivedData.size() - 1)));
        break;
    case MessageFlags::TargetFieldsReveal:
        //TODO
        ui->LstConsole->addItem(QString("TargetFieldReveal message."));
        PInfoWidget->WriteFieldsDataFromMapping(JSONFieldMediator::UnpackDataAsMappingInplace(ReceivedData.right(ReceivedData.size() - 1)));
        break;
    case MessageFlags::ReqInitPlayer:
        ui->LstConsole->addItem(QString("ReqInitPlayer message."));
        break;

    case MessageFlags::InitPlayerList:
        ui->LstConsole->addItem(QString("InitPlayerList message."));
        break;


    case MessageFlags::ResInitPlayer:
        ui->LstConsole->addItem(QString("ResInitPlayerOnServer message."));
        break;

    default:
        qDebug() << "Unhendled case!\n";
        break;
    }
}

void MainWindow::NewClientConnected(int Id, const QString& Nickname)
{
    //PlayerListView.append(Nickname);
    qDebug() << "Client with nickname:" << Nickname << " and Id" << Id << " has connected";
    //Model->setStringList(PlayerListView);
    //ui->LstConsole->addItem(Nickname);
}

void MainWindow::ClientDisconnected(int Id, const QString& Nickname)
{
    //PlayerListView.removeAll(Nickname);
    qDebug() << "Client with nickname:" << Nickname << " and Id" << Id << " has disconnected";
    //Model->setStringList(PlayerListView);
    //ui->LstConsole->addItem(Nickname);
}

void MainWindow::ServerUpdatePublicIP()
{
    //Now we could request new info.
    ui->LstConsole->clear();
    QString ServerInfo;
    ServerInfo = Manager->GetController()->GetServerRunInfo();
    ui->LstConsole->addItem(ServerInfo);
}

void MainWindow::SendPlayerInfoToTheServer(const QMap<QString, QString> &MappingToSend)
{
    Manager->SetFieldsToShare(MappingToSend);
    Manager->SendMessage(MessageFlags::BroadcastFieldsReveal);
}


void MainWindow::on_BtnClearConsole_clicked()
{
    ui->LstConsole->clear();
}


void MainWindow::on_BtnServerRun_clicked()
{
    DeviceController* Controller = Manager->GetController();
    if(Controller->IsConnected())
    {
        qDebug() << "Currently your session is in client mode. Disconnect from current server/device to be able to run server.";
        return;
    }
    if(Controller->IsServerStarted())
    {
        Controller->ShutdownServer();

        disconnect(Controller, &DeviceController::ServerReceivedData, this, &MainWindow::ClientDataReceived);
        //SetSessionManagerCallbacks(false);

        ui->connectedPlayersListView_Server->reset();
    }
    else
    {
        auto Port = ui->ServerPort->value();
        Controller->SetupTCPServer(Port);

        connect(Controller, &DeviceController::ServerReceivedData, this, &MainWindow::ClientDataReceived);
        //SetSessionManagerCallbacks(true);

        ui->connectedPlayersListView_Server->setModel(Manager->GetPlayerListViewModel());

        ServerUpdatePublicIP();
    }

    bool bIsServerRunning = Controller->IsServerStarted();
    QString ServerStatus = bIsServerRunning ? "1" : "0";
    ui->ServerIndicator->setProperty("state", ServerStatus);
    ui->BtnServerRun->setText(bIsServerRunning ? "Stop server" : "Run server");
    style()->polish(ui->ServerIndicator);
}


void MainWindow::on_BtnUpdateServInfo_clicked()
{
    if(Manager->GetController()->IsServerStarted())
    {
        ServerUpdatePublicIP();
    }
    else
    {
        ui->LstConsole->clear();
        ui->LstConsole->addItem("Currently not running server");
    }
}

void MainWindow::on_actionCheck_player_info_tab_triggered()
{
    PInfoWidget->show();
}


void MainWindow::on_backBtn_clicked()
{
    ui->mainWindowStackedWidget->setCurrentIndex(MainMenuWidgetIndexes::Home);
    ui->horizontalFrameHeader->setVisible(false);
}


void MainWindow::on_startServerPushButton_clicked()
{
    ui->mainWindowStackedWidget->setCurrentIndex(MainMenuWidgetIndexes::HostServer);
    ui->horizontalFrameHeader->setVisible(true);
}


void MainWindow::on_connectPushButton_clicked()
{
    ui->mainWindowStackedWidget->setCurrentIndex(MainMenuWidgetIndexes::ClientConnect);
    ui->horizontalFrameHeader->setVisible(true);
}


void MainWindow::on_exitPushButton_clicked()
{
    this->close();
}

void MainWindow::on_clientReadyButton_clicked()
{
    //TODO: make ready message to server.
}


void MainWindow::on_startGamePushButton_clicked()
{
    if(Manager->GetController()->IsServer())
    {
        Manager->PreStartGameSession();
    }
}

