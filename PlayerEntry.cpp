#include "PlayerEntry.h"

PlayerEntry::PlayerEntry() {}

PlayerEntry::PlayerEntry(QString Name)
{
    PlayerName = Name;
}

PlayerEntry::~PlayerEntry() {}

void PlayerEntry::InitPlayerEntry(const QVector<QString> &PlayerFieldsNames, QString DefaultValue)
{
    for(const QString& FieldName : PlayerFieldsNames)
    {
        PlayerFields.insert(FieldName, DefaultValue);
    }
}

void PlayerEntry::AddUpdateEntry(QPair<QString, QString> Pair)
{
    PlayerFields[Pair.first] = Pair.second;
}

void PlayerEntry::AddUpdateEntryBulk(const QMap<QString, QString> &Bulk)
{
    PlayerFields.insert(Bulk);
}

void PlayerEntry::SetPlayerName(QString NewName)
{
    PlayerName = NewName;
}

QVector<QString> PlayerEntry::GetPlayerFieldsNames()
{
    return PlayerFields.keys();
}

QVector<QString> PlayerEntry::GetPlayerFieldsValues()
{
    return PlayerFields.values();
}

QMap<QString, QString> PlayerEntry::GetPlayerFieldsCopy() const
{
    return PlayerFields;
}
