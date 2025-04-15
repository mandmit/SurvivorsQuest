#include "PublicIpAddressFetcher.h"

PublicIpAddressFetcher::PublicIpAddressFetcher(QObject *parent)
    : QObject(parent) {
    connect(&Manager, &QNetworkAccessManager::finished, this, &PublicIpAddressFetcher::onReplyFinished);
    QNetworkRequest request(QUrl("https://api64.ipify.org"));
    Manager.get(request);
}

void PublicIpAddressFetcher::RequestIpAddress()
{
    QNetworkRequest request(QUrl("https://api64.ipify.org"));
    Manager.get(request);
}

QString PublicIpAddressFetcher::GetIpAddress() const
{
    return PublicIPAddress.toString();
}

void PublicIpAddressFetcher::onReplyFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QString ipAddress = reply->readAll().trimmed();
        PublicIPAddress = QHostAddress(ipAddress);
        qDebug() << "Public IP Address:" << ipAddress;
    } else {
        PublicIPAddress = QHostAddress("0.0.0.0");
        qDebug() << "Error fetching IP:" << reply->errorString();
    }
    reply->deleteLater();
    emit IpAddressReceived();
}
