#ifndef PUBLICIPADDRESSFETCHER_H
#define PUBLICIPADDRESSFETCHER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QDebug>

class PublicIpAddressFetcher : public QObject
{
    Q_OBJECT
public:
    PublicIpAddressFetcher(QObject* parent = nullptr);

    void RequestIpAddress();
    QString GetIpAddress() const;

signals:
    void IpAddressReceived();

private slots:
    void onReplyFinished(QNetworkReply* reply);

private:
    QNetworkAccessManager NetManager;
    QHostAddress PublicIPAddress;
};

#endif // PUBLICIPADDRESSFETCHER_H
