#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QObject>
#include <QStringListModel>
#include "DeviceController.h"
#include "PlayerEntry.h"
#include "SessionUtilities.h"

class SessionManager : public QObject
{
    Q_OBJECT
public:
    explicit SessionManager(QObject* parent = nullptr);

    void StartGameSession();
    void InitPlayers();
    QByteArray GetFieldsToShareAndClear();
    void ChangePlayerName(const QString& NewName);
    void SetFieldsToShare(const QMap<QString, QString>& NewFields);
    void UpdatePlayerList(const QByteArray &Data);
    void SendMessage(MessageFlags Flag, QTcpSocket* Socket = nullptr/*const QByteArray& Payload = QByteArray()*/);
    QString GetPlayerNicknameById(int Id);
    QList<QString> GetPlayersList() const;
    DeviceController* GetController();
    QStringListModel* GetPlayerListViewModel();

private:

    void ProcessNewClientPlayerName(QTcpSocket* Sender, const QByteArray& Data);
    void UpdatePlayerListView();
    void ServerNotifyClientNameUpdated();

public slots:
    void ProcessServerData(QTcpSocket* Sender, const QByteArray& Data);
    void ProcessClientData(QTcpSocket* Sender, const QByteArray& Data);
    void SetupServerPlayer();
    void ClenupServerPlayer();
    void NewPlayerHandshake(int Id);
    void PlayerLeft(int Id);
    void ProcessServerConnection();
    void ProcessServerDisconnection();

signals:
    void ReceivedBroadcastedPlayerFields(int PlayerIndex, const QMap<QString, QString>& ReceivedFields);
    void NewPlayerNicknameReceived(int PlayerIndex, const QString& Nickname);
    void PlayerWithNicknameLeft(int PlayerIndex, const QString& Nickname);
    void RequestChangePlayerNickname();
    //void ChangeNicknameStatus(bool bIsSuccess);

private:
    int LocalPlayerId = -1;
    QMap<int, PlayerEntry> Players;//This info will be created by default for all players. Here each player could track fields of other players. Filled by server and opened for clients (sent when needed).
    QString PlayerName;
    QMap<int, QString> PlayerNameToUniqueId;//Initially it is filled by server and only when game is starting will be passed to the clients.
    QMap<QString, QString> FieldsToShare;//Should be updated when needed to send some particular fields and cleared when sent.

    QScopedPointer<QStringListModel> Model;
    QStringList PlayerListView;

    QScopedPointer<DeviceController> Controller;

};

#endif // SESSIONMANAGER_H
