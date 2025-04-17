#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "JsonFieldMediator.h"
#include "PlayerInfoWindow.h"
#include "SessionManager.h"
#include <QStringListModel>
#include <QStackedWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void SetupLocalPlayerName(const QString& PlayerName);

private:

    void SetDeviceControllerCallbacks();
    void SetSessionManagerCallbacks(bool bBindUnbind);

private slots:
    //Client slots
    void DeviceConnected();
    void DeviceDisconnected();
    void DeviceStateChanged(QAbstractSocket::SocketState);
    void DeviceErrorOccurred(QAbstractSocket::SocketError);
    void DeviceDataReady(QTcpSocket* Sender, const QByteArray& Data);
    void TriggerChangeNicknameWindow();
    //void CheckClientValidNickname(bool bIsSuccess);


    //Server slots
    void NewClientConnected(int Id, const QString& Nickname);
    void ClientDataReceived(QTcpSocket* Sender, const QByteArray& Data);
    void ClientDisconnected(int Id, const QString& Nickname);
    void ServerUpdatePublicIP();
    void SendPlayerInfoToTheServer(const QMap<QString, QString>& MappingToSend);

    void on_ClientIpAddress_textChanged(const QString &arg1);

    void on_BtnClearConsole_clicked();

    void on_BtnServerRun_clicked();

    void on_BtnUpdateServInfo_clicked();

    void on_actionCheck_player_info_tab_triggered();

    void on_backBtn_clicked();

    void on_startServerPushButton_clicked();

    void on_connectPushButton_clicked();

    void on_exitPushButton_clicked();

    void on_clientConnectBtn_clicked();

    void on_clientReadyButton_clicked();

private:
    Ui::MainWindow *ui;
    QScopedPointer<SessionManager> Manager;

    PlayerInfoWindow* PInfoWindow = nullptr;
    JSONFieldMediator JsonFieldConverter;

    //Test
    QStackedWidget *pages;

};
#endif // MAINWINDOW_H
