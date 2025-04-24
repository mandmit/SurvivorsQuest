#ifndef PLAYERINFOWIDGET_H
#define PLAYERINFOWIDGET_H

#include "PlayerEntry.h"
#include "ui_PlayerInfoWidget.h"
#include <QWidget>

namespace Ui
{
class PlayerInfoWidget;
}

class PlayerInfoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PlayerInfoWidget(QWidget *parent = nullptr);
    ~PlayerInfoWidget();

    void InitFieldsObjects();

    void WriteFieldsDataFromMapping(const QMap<QString, QString>& Mapping);
    void InitPlayer(const PlayerEntry& Player);

    QMap<QString, QString> ReadDataFromFields();

private:
    Ui::PlayerInfoWidget* ui;

    QMap<QString, QObject*> TagToObject;

signals:
    void UpdateButtonClicked(const QMap<QString, QString>& Mapping);

public slots:
    void UpdatePlayerInfoFields(const QMap<QString, QString>& Mapping);

private slots:
    void on_UpdatePlayerInfoFields_clicked();
};

#endif // PLAYERINFOWIDGET_H
