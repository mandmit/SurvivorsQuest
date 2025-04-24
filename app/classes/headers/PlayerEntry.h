#ifndef PLAYERENTRY_H
#define PLAYERENTRY_H

#include <QString>
#include <QMap>

struct PlayerFieldValue
{
    PlayerFieldValue();
    PlayerFieldValue(const QString& inFieldValue, const QString& DefValue = "???", bool bIsInitiallyRevealed = false);


    QString FieldValue;
    QString NonRevealedDefValue = "???";
    bool bIsRevealed = false;
};


class PlayerEntry
{
public:
    PlayerEntry();
    PlayerEntry(QString Name);
    ~PlayerEntry();

    //This version of function adds values for all given field names and marks them as non revealed ("???" when peeked). Suitable for initialization of player fields (owned by different players).
    void InitPlayerEntry(const QVector<QString>& PlayerFieldsNames, QString DefaultValue = "None");

    //This version of function adds values for all given field names and marks them as non revealed ("???" when peeked). Suitable for initialization of player fields (owned by different players).
    void InitPlayerEntry(const QMap<QString, QString>& Bulk);

    //Will return "None" if there is no valid value for given FieldName. Returns "???" by default if non revealed value or some valid QString value if revealed.
    QString PeekValueByFieldName(const QString& FieldName);

    //This function will add or update value of the given field name with field value (as string). Field counts as revealed by default when this function ends.
    void AddUpdateEntry(const QPair<QString, QString>& Pair, bool bShouldBeRevealed = false);

    //This version of function adds customizable PlayerFieldValue objects with properties (like already bIsRevealed, FieldName, FieldValue)
    void AddUpdateEntryBulk(const QMap<QString, PlayerFieldValue>& Bulk);

    //This version of function adds or updates pair of name to value in fields. Will be revealed if already inside mapping. Will be added as non reveaded if new.
    void AddUpdateEntryBulk(const QMap<QString, QString>& Bulk);

    //Passed by rvalue ref we can "move" data inside playerEntry object to avoid copying. Will clear prev mapping if any valid variables were stored inside.
    void AddUpdateEntryBulk(QMap<QString, PlayerFieldValue>&& Bulk);


    void SetPlayerName(QString NewName);

    QString GetPlayerName() const;

    QVector<QString> GetPlayerFieldsNames() const;

    QVector<PlayerFieldValue> GetPlayerFieldsValues() const;

    QVector<QString> GetPlayerFieldsValuesAsStrings() const;

    QMap<QString, PlayerFieldValue> GetPlayerFieldsCopy() const;

    QMap<QString, QString> GetPlayerFieldsCopyAsStrings() const;

private:

    QString PlayerName;

    QMap<QString, PlayerFieldValue> PlayerFields;
};

#endif // PLAYERENTRY_H
