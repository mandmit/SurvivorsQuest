#include "PlayerInfoWindow.h"
#include "qlabel.h"
#include "ui_PlayerInfoWindow.h"

PlayerInfoWindow::PlayerInfoWindow(QWidget *parent) : QWidget(parent), ui(new Ui::PlayerInfoWindow) {
    ui->setupUi(this);


}

PlayerInfoWindow::~PlayerInfoWindow()
{
    delete ui;
}

void PlayerInfoWindow::InitFieldsObjects()
{
    const QList<QObject*> allWidgets = findChildren<QObject*>(); // Get all child widgets
    for (QObject* obj : allWidgets) {
        if (obj->property("FieldTag").isValid()) {
            TagToObject.insert(obj->property("FieldTag").value<QString>(), obj);
        }
    }
}

void PlayerInfoWindow::WriteFieldsDataFromMapping(const QMap<QString, QString> &Mapping)
{
    for(auto it = Mapping.begin(); it != Mapping.end(); ++it)
    {
        if(TagToObject.contains(it.key()))
        {
            //Questionable - only works with plain text edit objects.
            QPlainTextEdit* EditText = dynamic_cast<QPlainTextEdit*>(TagToObject[it.key()]);
            if(EditText)
            {
                EditText->setPlainText(it.value());
            }
        }
    }
}

void PlayerInfoWindow::InitPlayer(const PlayerEntry &Player)
{
    QMap<QString, QString> Fields = Player.GetPlayerFieldsCopy();
    int i = 0;
    for(auto It = Fields.begin(); It != Fields.end(); ++It)
    {
        QHBoxLayout* ItemLayout = new QHBoxLayout(ui->PlayerFieldsGroupBox);
        ui->PlayerFieldsVerticalLayout->addLayout(ItemLayout);

        QString LayoutName = "PlayerFieldHorizontalLayout_";
        LayoutName.append(QString::number(i++));

        ItemLayout->setObjectName(LayoutName);

        QLabel* ItemLabel = new QLabel(ui->PlayerFieldsGroupBox);
        QString LabelName = It.key();
        LabelName.push_back(":");
        ItemLabel->setText(It.key());

        ItemLayout->addWidget(ItemLabel);
        QPlainTextEdit* TextEditField = new QPlainTextEdit(ui->PlayerFieldsGroupBox);
        TextEditField->setProperty("FieldTag", QVariant(QCoreApplication::translate("PlayerInfoWindow", It.key().toUtf8().constData(), nullptr)));
        TextEditField->setPlainText(It.value());
        ItemLayout->addWidget(TextEditField);
    }
}

QMap<QString, QString> PlayerInfoWindow::ReadDataFromFields()
{
    QMap<QString, QString> Result;
    for(auto it = TagToObject.begin(); it != TagToObject.end(); ++it)
    {
        QPlainTextEdit* EditText = dynamic_cast<QPlainTextEdit*>(it.value());
        if(EditText)
        {
            Result.insert(it.key(), EditText->toPlainText());
        }
    }
    return Result;
}

void PlayerInfoWindow::on_UpdatePlayerInfoFields_clicked()
{
    //Temp name. Only for test
    emit UpdateButtonClicked(ReadDataFromFields());
}

void PlayerInfoWindow::UpdatePlayerInfoFields(const QMap<QString, QString>& Mapping)
{
    WriteFieldsDataFromMapping(Mapping);
}

