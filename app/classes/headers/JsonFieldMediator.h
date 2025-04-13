#ifndef JSONFIELDMEDIATOR_H
#define JSONFIELDMEDIATOR_H

#include <QObject>


#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

class JSONFieldMediator
{
public:
    explicit JSONFieldMediator();

    void PackDataAsMapping(const QMap<QString, QString>& FieldToValueMapping);
    void PackDataAsString(const QString& DataToPack);
    void PackDataAsByteArray(const QByteArray& DataToPack);
    static QMap<QString, QString> UnpackDataAsMappingInplace(const QByteArray& InputData);
    static QMap<QString, QString> UnpackDataAsMappingInplace(const QJsonDocument& Doc);
    static QByteArray PackDataAsMappingInplace(const QMap<QString, QString>& FieldToValueMapping);


    QByteArray GetJsonByteArray() const;
    QJsonDocument GetPackedJsonDocument() const;

private:

    QJsonDocument PackedData;
    QByteArray JSONByteArray;
signals:
};

#endif // JSONFIELDMEDIATOR_H
