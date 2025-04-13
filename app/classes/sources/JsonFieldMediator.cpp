#include "JsonFieldMediator.h"

JSONFieldMediator::JSONFieldMediator(){}

void JSONFieldMediator::PackDataAsMapping(const QMap<QString, QString> &FieldToValueMapping)
{
    QJsonObject Object;
    for(auto it = FieldToValueMapping.begin(); it != FieldToValueMapping.end(); ++it)
    {
        Object.insert(it.key(), it.value());
    }
    PackedData.setObject(Object);
    JSONByteArray = PackedData.toJson(QJsonDocument::Compact);
}

void JSONFieldMediator::PackDataAsString(const QString &DataToPack)
{
    PackedData = QJsonDocument::fromJson(DataToPack.toUtf8());
    JSONByteArray = PackedData.toJson(QJsonDocument::Compact);
}

void JSONFieldMediator::PackDataAsByteArray(const QByteArray &DataToPack)
{
    PackedData = QJsonDocument::fromJson(DataToPack);
    JSONByteArray = PackedData.toJson(QJsonDocument::Compact);
}

QMap<QString, QString> JSONFieldMediator::UnpackDataAsMappingInplace(const QByteArray& InputData)
{
    QJsonDocument Doc = QJsonDocument::fromJson(InputData);
    return UnpackDataAsMappingInplace(Doc);
}

QMap<QString, QString> JSONFieldMediator::UnpackDataAsMappingInplace(const QJsonDocument &Doc)
{
    QMap<QString, QString> Result;
    if(!Doc.isNull())
    {
        if(Doc.isObject())
        {
            QJsonObject JsonObj = Doc.object();
            for(auto it = JsonObj.begin(); it != JsonObj.end(); ++it)
            {
                if(it.value().isString())
                {
                    Result.insert(it.key(), it.value().toString("Null"));
                }
            }
        }
    }
    return Result;
}

QByteArray JSONFieldMediator::PackDataAsMappingInplace(const QMap<QString, QString> &FieldToValueMapping)
{
    QJsonDocument Doc;

    QJsonObject Object;
    for(auto it = FieldToValueMapping.begin(); it != FieldToValueMapping.end(); ++it)
    {
        Object.insert(it.key(), it.value());
    }
    Doc.setObject(Object);
    QByteArray Array = Doc.toJson(QJsonDocument::Compact);
    return Array;
}

QByteArray JSONFieldMediator::GetJsonByteArray() const
{
    return JSONByteArray;
}

QJsonDocument JSONFieldMediator::GetPackedJsonDocument() const
{
    return PackedData;
}
