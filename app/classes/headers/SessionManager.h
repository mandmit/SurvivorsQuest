#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QObject>
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
    void InitLocalPlayer(QString NewName);
    void SetFieldsToShare(const QMap<QString, QString>& NewFields);
    void UpdatePlayerList(const QByteArray &Data);
    void SendMessage(MessageFlags Flag, QTcpSocket* Socket = nullptr/*const QByteArray& Payload = QByteArray()*/);

private:

    void ProcessNewPlayerName(QTcpSocket* Sender, QByteArray Data);

public slots:
    void ProcessServerData(QTcpSocket* Sender, QByteArray Data);
    void ProcessClientData(QTcpSocket* Sender, QByteArray Data);
    void SetupServerPlayer();
    void ClenupServerPlayer();
    void NewPlayerHandshake(int Id);
    void PlayerLeft(int Id);

signals:
    void ReceivedBroadcastedPlayerFields(int PlayerIndex, const QMap<QString, QString>& ReceivedFields);

public:
    int LocalPlayerId = -1;
    QMap<int, PlayerEntry> Players;//This info will be created by default for all players. Here each player could track fields of other players. Filled by server and opened for clients (sent when needed).
    QString PlayerName;
    QMap<int, QString> PlayerNameToUniqueId;//Initially it is filled by server and only when game is starting will be passed to the clients.
    QMap<QString, QString> FieldsToShare;//Should be updated when needed to send some particular fields and cleared when sent.

    DeviceController* Controller;

};

#endif // SESSIONMANAGER_H
