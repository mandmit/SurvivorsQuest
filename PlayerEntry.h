#ifndef PLAYERENTRY_H
#define PLAYERENTRY_H

#include <QString>
#include <QMap>

class PlayerEntry
{
public:
    PlayerEntry();
    PlayerEntry(QString Name);
    ~PlayerEntry();

    void InitPlayerEntry(const QVector<QString>& PlayerFieldsNames, QString DefaultValue = "None");
    void AddUpdateEntry(QPair<QString, QString> Pair);
    void AddUpdateEntryBulk(const QMap<QString, QString>& Bulk);
    void SetPlayerName(QString NewName);
    QVector<QString> GetPlayerFieldsNames();
    QVector<QString> GetPlayerFieldsValues();
    QMap<QString, QString> GetPlayerFieldsCopy() const;

private:

    QString PlayerName;

    QMap<QString, QString> PlayerFields;
};

#endif // PLAYERENTRY_H
