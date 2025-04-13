#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "JsonFieldMediator.h"
#include "PlayerInfoWindow.h"
#include "DeviceController.h"
#include "SessionManager.h"
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

    void SetupPlayerName(QString PlayerName);

private:

    void SetDeviceController();

private slots:
    void on_ClientIpAddress_textChanged(const QString &arg1);

    void on_BtnConnect_clicked();

    //Client slots
    void DeviceConnected();
    void DeviceDisconnected();
    void DeviceStateChanged(QAbstractSocket::SocketState);
    void DeviceErrorOccurred(QAbstractSocket::SocketError);
    void DeviceDataReady(QTcpSocket* Sender, QByteArray);

    //Server slots
    void NewClientConnected(int Id);
    void ClientDataReceived(QTcpSocket* Sender, QByteArray);
    void ClientDisconnected(int Id);
    void ServerUpdatePublicIP();
    void SendPlayerInfoToTheServer(const QMap<QString, QString>& MappingToSend);


    void on_BtnClearConsole_clicked();

    void on_BtnClientSend_clicked();

    void on_BtnServerRun_clicked();

    void on_BtnUpdateServInfo_clicked();

    void on_BtnServerSend_clicked();

    void on_actionCheck_player_info_tab_triggered();

private:
    Ui::MainWindow *ui;
    SessionManager* Manager;

    PlayerInfoWindow* PInfoWindow = nullptr;
    JSONFieldMediator JsonFieldConverter;

    //Test
    QStackedWidget *pages;

};
#endif // MAINWINDOW_H
