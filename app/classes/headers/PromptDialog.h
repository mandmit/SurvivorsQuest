#ifndef PROMPTDIALOG_H
#define PROMPTDIALOG_H

#include <QObject>
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>

class PromptDialog : public QDialog {
    Q_OBJECT

public:
    PromptDialog(QWidget* parent = nullptr);
    QString GetText() const;

private:
    QLineEdit* InputField;
};

#endif // PROMPTDIALOG_H
