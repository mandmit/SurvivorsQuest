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
        PlayerFields.insert(FieldName, {DefaultValue});
    }
}

void PlayerEntry::InitPlayerEntry(const QMap<QString, QString> &Bulk)
{
    for(auto It = Bulk.begin(); It != Bulk.end(); ++It)
    {
        PlayerFields[It.key()] = {It.value()};//Will call constructor with one parameter (given value) and assign as non revealed ("???" by default)
    }
}

QString PlayerEntry::PeekValueByFieldName(const QString &FieldName)
{
    if(PlayerFields.contains(FieldName))
    {
        PlayerFieldValue& FieldValueRef = PlayerFields[FieldName];
        return (FieldValueRef.bIsRevealed) ? (FieldValueRef.FieldValue) : (FieldValueRef.NonRevealedDefValue);
    }
    return "None";
}

void PlayerEntry::AddUpdateEntry(const QPair<QString, QString>& Pair, bool bShouldBeRevealed)
{
    PlayerFieldValue& PlayerFieldValueRef = PlayerFields[Pair.first];//If there is no such variable - will create new one.
    if(!Pair.second.isNull() && !Pair.second.isEmpty())
    {
        PlayerFieldValueRef.bIsRevealed = bShouldBeRevealed;
        PlayerFieldValueRef.FieldValue = Pair.second;
    }
}

void PlayerEntry::AddUpdateEntryBulk(const QMap<QString, PlayerFieldValue> &Bulk)
{
    PlayerFields.insert(Bulk);
}

void PlayerEntry::AddUpdateEntryBulk(const QMap<QString, QString> &Bulk)
{
    for(auto It = Bulk.begin(); It != Bulk.end(); ++It)
    {
        if(PlayerFields.contains(It.key()))
        {
            PlayerFieldValue& FieldValueRef = PlayerFields[It.key()];
            FieldValueRef.bIsRevealed = true;
            FieldValueRef.FieldValue = It.value();
        }
        else
        {
            //Maybe I should also reveal this variable when adding new one?
            PlayerFields[It.key()] = {It.value()};//Will call constructor with one parameter (given value) and assign as non revealed ("???" by default)
        }
    }
}

void PlayerEntry::AddUpdateEntryBulk(QMap<QString, PlayerFieldValue> &&Bulk)
{
    if(!PlayerFields.isEmpty())
    {
        PlayerFields.clear();
    }
    PlayerFields = Bulk;
}

void PlayerEntry::SetPlayerName(QString NewName)
{
    PlayerName = NewName;
}

QString PlayerEntry::GetPlayerName() const
{
    return PlayerName;
}

QVector<QString> PlayerEntry::GetPlayerFieldsNames() const
{
    return PlayerFields.keys();
}

QVector<PlayerFieldValue> PlayerEntry::GetPlayerFieldsValues() const
{
    return PlayerFields.values();
}

QVector<QString> PlayerEntry::GetPlayerFieldsValuesAsStrings() const
{
    QVector<QString> ValuesCopy;
    ValuesCopy.reserve(PlayerFields.size());
    for(auto It = PlayerFields.begin(); It != PlayerFields.end(); ++It)
    {
        ValuesCopy.push_back(It.value().FieldValue);
    }
    return ValuesCopy;
}

QMap<QString, PlayerFieldValue> PlayerEntry::GetPlayerFieldsCopy() const
{
    return PlayerFields;
}

QMap<QString, QString> PlayerEntry::GetPlayerFieldsCopyAsStrings() const
{
    QMap<QString, QString> MappingCopy;
    for(auto It = PlayerFields.begin(); It != PlayerFields.end(); ++It)
    {
        MappingCopy.insert(It.key(), It.value().FieldValue);
    }
    return MappingCopy;
}

//PlayerFieldValue Struct functionality

PlayerFieldValue::PlayerFieldValue()
{
    FieldValue = "Non specified value";
}

PlayerFieldValue::PlayerFieldValue(const QString &inFieldValue, const QString &DefValue, bool bIsInitiallyRevealed)
{
    FieldValue = inFieldValue;
    NonRevealedDefValue = DefValue;
    bIsRevealed = bIsInitiallyRevealed;
}
