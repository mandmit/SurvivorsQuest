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
    QString getText() const;

private:
    QLineEdit* inputField;
};

#endif // PROMPTDIALOG_H
