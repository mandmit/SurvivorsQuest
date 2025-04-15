#include "mainwindow.h"
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

    ui->connectedPlayersListView->setModel(Manager->GetPlayerListViewModel());


    if (!PInfoWindow) { // Ensure only one instance exists
        //PInfoWindow = new PlayerInfoWindow();
        //connect(PInfoWindow, &PlayerInfoWindow::UpdateButtonClicked, this, &MainWindow::SendPlayerInfoToTheServer);


        //Tempo Only test

        // PlayerEntry PlayerEntry;
        // PlayerEntry.InitPlayerEntry({"Age", "Name", "Ocupation"}, "Just tempon variable");
        // PInfoWindow->InitPlayer(PlayerEntry);
    }
    ui->mainWindowStackedWidget->setCurrentIndex(MainMenuWidgetIndexes::Home);
    ui->horizontalFrameHeader->setVisible(false);

}

MainWindow::~MainWindow()
{
    delete ui;
    if (PInfoWindow) {
        delete PInfoWindow;
    }
}

void MainWindow::SetupPlayerName(QString PlayerName)
{
    Manager->InitLocalPlayer(PlayerName);
}

void MainWindow::SetDeviceControllerCallbacks()
{
    connect(Manager->GetController(), &DeviceController::ConnectedToServer, this, &MainWindow::DeviceConnected);
    connect(Manager->GetController(), &DeviceController::DisconnectedFromServer, this, &MainWindow::DeviceDisconnected);
    connect(Manager->GetController(), &DeviceController::StateChanged, this, &MainWindow::DeviceStateChanged);
    connect(Manager->GetController(), &DeviceController::ErrorOccurred, this, &MainWindow::DeviceErrorOccurred);
    connect(Manager->GetController(), &DeviceController::ClientReceivedData, this, &MainWindow::DeviceDataReady);

    connect(&Manager->GetController()->fetcher, &PublicIpAddressFetcher::IpAddressReceived, this, &MainWindow::ServerUpdatePublicIP);
}

void MainWindow::SetSessionManagerCallbacks(bool bBindUnbind)
{
    if(bBindUnbind)
    {
        connect(Manager.get(), &SessionManager::NewPlayerNicknameReceived, this, &MainWindow::NewClientConnected);
        connect(Manager.get(), &SessionManager::PlayerWithNicknameLeft, this, &MainWindow::ClientDisconnected);
    }
    else
    {
        disconnect(Manager.get(), &SessionManager::NewPlayerNicknameReceived, this, &MainWindow::NewClientConnected);
        disconnect(Manager.get(), &SessionManager::PlayerWithNicknameLeft, this, &MainWindow::ClientDisconnected);
    }
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

void MainWindow::DeviceConnected()
{
    ui->LstConsole->addItem("Connected to Server");
    ui->clientConnectBtn->setText("Disconnect");
}

void MainWindow::DeviceDisconnected()
{
    ui->LstConsole->addItem("Disconnected from Server");
    ui->clientConnectBtn->setText("Connect");
}

void MainWindow::DeviceStateChanged(QAbstractSocket::SocketState state)
{
    QMetaEnum MetaEnum = QMetaEnum::fromType<QAbstractSocket::SocketState>();
    ui->LstConsole->addItem(MetaEnum.valueToKey(state));
}

void MainWindow::DeviceErrorOccurred(QAbstractSocket::SocketError error)
{
    QMetaEnum MetaEnum = QMetaEnum::fromType<QAbstractSocket::SocketError>();
    ui->LstConsole->addItem(MetaEnum.valueToKey(error));
}

void MainWindow::DeviceDataReady(QTcpSocket* Sender, QByteArray data)
{
    ui->LstConsole->addItem(QString(data));
}


void MainWindow::ClientDataReceived(QTcpSocket* Sender, QByteArray receivedData)//Data received by current device
{
    ui->LstConsole->addItem(QString(receivedData));
    MessageFlags Flag = static_cast<MessageFlags>(receivedData.at(0));
    switch(Flag)
    {
    case MessageFlags::BroadcastFieldsReviel:
        ui->LstConsole->addItem(QString("BroadcastFieldReviel message."));
        PInfoWindow->WriteFieldsDataFromMapping(JSONFieldMediator::UnpackDataAsMappingInplace(receivedData.right(receivedData.size() - 1)));
        break;
    case MessageFlags::TargetFieldsReviel:
        //TODO
        ui->LstConsole->addItem(QString("TargetFieldReviel message."));
        PInfoWindow->WriteFieldsDataFromMapping(JSONFieldMediator::UnpackDataAsMappingInplace(receivedData.right(receivedData.size() - 1)));
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
    Manager->SendMessage(MessageFlags::BroadcastFieldsReviel);
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
        SetSessionManagerCallbacks(false);
    }
    else
    {
        auto Port = ui->ServerPort->value();
        Controller->SetupTCPServer(Port);

        connect(Controller, &DeviceController::ServerReceivedData, this, &MainWindow::ClientDataReceived);
        SetSessionManagerCallbacks(true);

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
    PInfoWindow->show();
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
