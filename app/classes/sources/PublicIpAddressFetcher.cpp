#include "PublicIpAddressFetcher.h"

PublicIpAddressFetcher::PublicIpAddressFetcher(QObject *parent)
    : QObject(parent) {
    connect(&manager, &QNetworkAccessManager::finished, this, &PublicIpAddressFetcher::onReplyFinished);
    QNetworkRequest request(QUrl("https://api64.ipify.org")); // or use https://checkip.amazonaws.com
    manager.get(request);
}

void PublicIpAddressFetcher::RequestIpAddress()
{
    QNetworkRequest request(QUrl("https://api64.ipify.org"));
    manager.get(request);
}

QString PublicIpAddressFetcher::GetIpAddress() const
{
    return publicIPAddress.toString();
}

void PublicIpAddressFetcher::onReplyFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QString ipAddress = reply->readAll().trimmed();
        publicIPAddress = QHostAddress(ipAddress);
        qDebug() << "Public IP Address:" << ipAddress;
    } else {
        publicIPAddress = QHostAddress("0.0.0.0");
        qDebug() << "Error fetching IP:" << reply->errorString();
    }
    reply->deleteLater();
    emit IpAddressReceived();
}
