#include "PlayerInfoWidget.h"
#include <QLabel>
#include <QPlainTextEdit>
#include "ui_PlayerInfoWidget.h"

PlayerInfoWidget::PlayerInfoWidget(QWidget *parent) : QWidget(parent), ui(new Ui::PlayerInfoWidget) {
    ui->setupUi(this);


}

PlayerInfoWidget::~PlayerInfoWidget()
{
    delete ui;
}

void PlayerInfoWidget::InitFieldsObjects()
{
    const QList<QObject*> allWidgets = findChildren<QObject*>(); // Get all child widgets
    for (QObject* obj : allWidgets) {
        if (obj->property("FieldTag").isValid()) {
            TagToObject.insert(obj->property("FieldTag").value<QString>(), obj);
        }
    }
}

void PlayerInfoWidget::WriteFieldsDataFromMapping(const QMap<QString, QString> &Mapping)
{
    for(auto It = Mapping.begin(); It != Mapping.end(); ++It)
    {
        if(TagToObject.contains(It.key()))
        {
            //Questionable - only works with plain text edit objects.
            QPlainTextEdit* EditText = dynamic_cast<QPlainTextEdit*>(TagToObject[It.key()]);
            if(EditText)
            {
                EditText->setPlainText(It.value());
            }
        }
    }
}

void PlayerInfoWidget::InitPlayer(const PlayerEntry &Player)
{
    if(!TagToObject.isEmpty())
    {
        TagToObject.clear();
    }

    QMap<QString, QString> Fields = Player.GetPlayerFieldsCopyAsStrings();
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
        TextEditField->setProperty("FieldTag", QVariant(QCoreApplication::translate("PlayerInfoWidget", It.key().toUtf8().constData(), nullptr)));
        TextEditField->setPlainText(It.value());
        ItemLayout->addWidget(TextEditField);

        TagToObject.insert(It.key(), TextEditField);
    }
}

QMap<QString, QString> PlayerInfoWidget::ReadDataFromFields()
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

void PlayerInfoWidget::on_UpdatePlayerInfoFields_clicked()
{
    //Temp name. Only for test
    emit UpdateButtonClicked(ReadDataFromFields());
}

void PlayerInfoWidget::UpdatePlayerInfoFields(const QMap<QString, QString>& Mapping)
{
    WriteFieldsDataFromMapping(Mapping);
}

