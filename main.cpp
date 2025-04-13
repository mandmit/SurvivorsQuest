#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QFile>
#include "PromptDialog.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile file(":/MainStyle");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QString style = QString::fromUtf8(file.readAll());
        a.setStyleSheet(style);
    }


    PromptDialog prompt;
    if (prompt.exec() == QDialog::Accepted) {
        QString Name = prompt.getText();

        // Pass the name to your main window, or use it however you need
        QTranslator translator;
        const QStringList uiLanguages = QLocale::system().uiLanguages();
        for (const QString &locale : uiLanguages) {
            const QString baseName = "SurvivorsQuest_" + QLocale(locale).name();
            if (translator.load(":/i18n/" + baseName)) {
                a.installTranslator(&translator);
                break;
            }
        }
        MainWindow w;
        w.setWindowTitle("Survivors Quest");
        w.setWindowIcon(QIcon(":/Icon/MainIcon"));
        w.SetupPlayerName(Name);
        w.show();


        return a.exec();
    }

    return 0;
}
