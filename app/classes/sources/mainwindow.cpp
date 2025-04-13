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
    Manager = new SessionManager(this);
    SetDeviceController();
    if (!PInfoWindow) { // Ensure only one instance exists
        PInfoWindow = new PlayerInfoWindow();
        connect(PInfoWindow, &PlayerInfoWindow::UpdateButtonClicked, this, &MainWindow::SendPlayerInfoToTheServer);


        //Tempo Only test

        // PlayerEntry PlayerEntry;
        // PlayerEntry.InitPlayerEntry({"Age", "Name", "Ocupation"}, "Just tempon variable");
        // PInfoWindow->InitPlayer(PlayerEntry);
    }

    // QWidget *central = new QWidget;
    // QVBoxLayout *layout = new QVBoxLayout(central);

    // // Page selector buttons
    // QPushButton *btnHome = new QPushButton("Home");
    // QPushButton *btnSettings = new QPushButton("Settings");

    // // Pages
    // QWidget *homePage = new QWidget;
    // homePage->setLayout(new QVBoxLayout);
    // homePage->layout()->addWidget(new QLabel("Welcome to Home"));

    // QWidget *settingsPage = new QWidget;
    // settingsPage->setLayout(new QVBoxLayout);
    // settingsPage->layout()->addWidget(new QLabel("Game Settings"));

    // // Stacked widget
    // pages = new QStackedWidget;
    // pages->addWidget(homePage);      // Index 0
    // pages->addWidget(settingsPage);  // Index 1

    // // Connect buttons to page switching
    // connect(btnHome, &QPushButton::clicked, this, [=]() {
    //     pages->setCurrentIndex(0);
    // });

    // connect(btnSettings, &QPushButton::clicked, this, [=]() {
    //     pages->setCurrentIndex(1);
    // });

    // // Add widgets to layout
    // layout->addWidget(btnHome);
    // layout->addWidget(btnSettings);
    // layout->addWidget(pages);

    // setCentralWidget(central);

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

void MainWindow::SetDeviceController()
{
    connect(Manager->Controller, &DeviceController::ConnectedToServer, this, &MainWindow::DeviceConnected);
    connect(Manager->Controller, &DeviceController::DisconnectedFromServer, this, &MainWindow::DeviceDisconnected);
    connect(Manager->Controller, &DeviceController::StateChanged, this, &MainWindow::DeviceStateChanged);
    connect(Manager->Controller, &DeviceController::ErrorOccurred, this, &MainWindow::DeviceErrorOccurred);
    connect(Manager->Controller, &DeviceController::ClientReceivedData, this, &MainWindow::DeviceDataReady);

    connect(&Manager->Controller->fetcher, &PublicIpAddressFetcher::IpAddressReceived, this, &MainWindow::ServerUpdatePublicIP);
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


void MainWindow::on_BtnConnect_clicked()
{

    if(Manager->Controller->IsServerStarted())
    {
        qDebug() << "Currently your session is in server mode. Shutdown server to be able to connect as an client.";
        return;
    }
    if(Manager->Controller->IsConnected())
    {
        Manager->Controller->DisconnectFromServer();
    }
    else
    {
        auto Ip = ui->ClientIpAddress->text();
        auto Port = ui->ClientPort->value();
        Manager->Controller->ConnectToServer(Ip, Port);
    }
}

void MainWindow::DeviceConnected()
{
    ui->LstConsole->addItem("Connected to Device");
    ui->BtnConnect->setText("Disconnect");
    ui->ClientSendData_GroupBox->setEnabled(true);
}

void MainWindow::DeviceDisconnected()
{
    ui->LstConsole->addItem("Disconnected from Device");
    ui->BtnConnect->setText("Connect");
    ui->ClientSendData_GroupBox->setEnabled(false);
    ui->ClientTextToSend->clear();
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

void MainWindow::NewClientConnected(int Id)
{
    QString Str("Client");
    Str.append(QString::number(Id));
    Str.append(" connected");
    ui->LstConsole->addItem(Str);
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

void MainWindow::ClientDisconnected(int Id)
{
    QString Str("Client");
    Str.append(QString::number(Id));
    Str.append(" disconnected");
    ui->LstConsole->addItem(Str);
}

void MainWindow::ServerUpdatePublicIP()
{
    //Now we could request new info.
    QString ServerInfo;
    ServerInfo = Manager->Controller->GetServerRunInfo();
    ui->ServerRunInfo->setPlainText(ServerInfo);
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


void MainWindow::on_BtnClientSend_clicked()
{
    Manager->Controller->SendDataToServer(ui->ClientTextToSend->text().trimmed().toUtf8());
}


void MainWindow::on_BtnServerRun_clicked()
{
    if(Manager->Controller->IsConnected())
    {
        qDebug() << "Currently your session is in client mode. Disconnect from current server/device to be able to run server.";
        return;
    }
    if(Manager->Controller->IsServerStarted())
    {
        Manager->Controller->ShutdownServer();
        disconnect(Manager->Controller, &DeviceController::NewClientConnected, this, &MainWindow::NewClientConnected);
        disconnect(Manager->Controller, &DeviceController::ServerReceivedData, this, &MainWindow::ClientDataReceived);
        disconnect(Manager->Controller, &DeviceController::OldClientDisconnected, this, &MainWindow::ClientDisconnected);
        ui->ServerRunInfo->clear();
        ui->ServerSendData_GroupBox->setEnabled(false);
    }
    else
    {
        auto Port = ui->ServerPort->value();
        Manager->Controller->SetupTCPServer(Port);
        connect(Manager->Controller, &DeviceController::NewClientConnected, this, &MainWindow::NewClientConnected);
        connect(Manager->Controller, &DeviceController::ServerReceivedData, this, &MainWindow::ClientDataReceived);
        connect(Manager->Controller, &DeviceController::OldClientDisconnected, this, &MainWindow::ClientDisconnected);

        //I need to receive this info when server is running, not instantly after request for run.
        // QString ServerInfo;
        // ServerInfo = _Controller->GetServerRunInfo();
        // ui->ServerRunInfo->setPlainText(ServerInfo);
        ui->ServerSendData_GroupBox->setEnabled(true);

    }

    bool bIsServerRunning = Manager->Controller->IsServerStarted();
    QString ServerStatus = bIsServerRunning ? "1" : "0";
    ui->ServerIndicator->setProperty("state", ServerStatus);
    ui->BtnServerRun->setText(bIsServerRunning ? "Stop server" : "Run server");
    style()->polish(ui->ServerIndicator);
}


void MainWindow::on_BtnUpdateServInfo_clicked()
{
    if(Manager->Controller->IsServerStarted())
    {
        QString ServerInfo;
        ServerInfo = Manager->Controller->GetServerRunInfo();
        ui->ServerRunInfo->setPlainText(ServerInfo);
    }
    else
    {
        ui->ServerRunInfo->clear();
    }
}


void MainWindow::on_BtnServerSend_clicked()
{
    Manager->Controller->BroadcastToAllClients(ui->ServerText->text().toUtf8());
}


void MainWindow::on_actionCheck_player_info_tab_triggered()
{
    PInfoWindow->show();
}

