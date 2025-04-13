#ifndef PLAYERINFOWINDOW_H
#define PLAYERINFOWINDOW_H

#include "PlayerEntry.h"
#include "ui_PlayerInfoWindow.h"
#include <QWidget>

namespace Ui
{
class PlayerInfoWindow;
}

class PlayerInfoWindow : public QWidget
{
    Q_OBJECT
public:
    explicit PlayerInfoWindow(QWidget *parent = nullptr);
    ~PlayerInfoWindow();

    void InitFieldsObjects();

    void WriteFieldsDataFromMapping(const QMap<QString, QString>& Mapping);
    void InitPlayer(const PlayerEntry& Player);

    QMap<QString, QString> ReadDataFromFields();

private:
    Ui::PlayerInfoWindow* ui;

    QMap<QString, QObject*> TagToObject;

signals:
    void UpdateButtonClicked(const QMap<QString, QString>& Mapping);

public slots:
    void UpdatePlayerInfoFields(const QMap<QString, QString>& Mapping);

private slots:
    void on_UpdatePlayerInfoFields_clicked();
};

#endif // PLAYERINFOWINDOW_H
