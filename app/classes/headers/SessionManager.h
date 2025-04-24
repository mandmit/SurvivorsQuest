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

    void PreStartGameSession();
    void InitPlayers();
    QByteArray GetFieldsToShareAndClear();
    void ChangePlayerName(const QString& NewName);
    void SetFieldsToShare(const QMap<QString, QString>& NewFields);
    void UpdatePlayerList(const QByteArray &Data);
    void SendMessage(MessageFlags Flag, QTcpSocket* Socket = nullptr/*const QByteArray& Payload = QByteArray()*/);
    QString GetPlayerNicknameById(int Id);
    QVector<PlayerEntry*> GetPlayerEntriesList();
    QList<QString> GetPlayersListNames() const;
    DeviceController* GetController();
    PlayerEntry* GetLocalPlayerEntry();
    QStringListModel* GetPlayerListViewModel();
    void SetFieldsForSession(const QList<QString>& Fields, const QString& DefaultValue = "???");

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
    void StartLocalSession();
    void RequestChangePlayerNickname();

private:
    int LocalPlayerId = -1;
    QMap<int, PlayerEntry> Players;//This info will be created by default for all players. Here each player could track fields of other players. Filled by server and opened for clients (sent when needed).
    QString PlayerName;

    PlayerEntry* LocalPlayer = nullptr;

    QMap<int, QString> PlayerNameToUniqueId;//Initially it is filled by server and only when game is starting will be passed to the clients.
    QMap<QString, QString> FieldsToShare;//Should be updated when needed to send some particular fields and cleared when sent.

    QScopedPointer<QStringListModel> Model;
    QStringList PlayerListView;

    QScopedPointer<DeviceController> Controller;

    //Session/game props
    QMap<QString, QVector<QString>> DefaultFieldToValuesMapping
        {
        {"Age", {}}, {"Sex", {"Male", "Female"}},
        {"Occupation/Skills", {"Doctor","Mechanic", "Teacher"}}, {"Inventory", {"Knife", "Pistol", "First Aid Kit", "Battery", "Food Rations (3 days)"}},
        {"Personal Traits", {"Loyal", "Paranoid", "Leader", "Selfish", "Antisocial", "Empathic"}}, {"Flaws", {"Injured", "Blind", "Deaf", "Low IQ"}},
        {"Physical Health", {"Athletic", "Avarage", "Weak"}}, {"Mental Health", {"Normal", "Crazy", "Emotionless"}},
        {"Criminal Records", {"Killer", "Thief", "Hacker"}}
        };

    QList<QString> FieldsForGame;



};

#endif // SESSIONMANAGER_H
