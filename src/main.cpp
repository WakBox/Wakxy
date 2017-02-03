#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Check the settings.ini for the language
    QSettings settings("settings.ini", QSettings::IniFormat);
    QString Lang = settings.value("Language/locale").toString();
    QString Language;

    // If a language is set in the settings.ini file : default is English
    if(Lang == "fr") {
        Language = "Wakxy_fr";
    }

    // Now, set the language
    QTranslator translator;
    translator.load(Language);
    a.installTranslator(&translator);

    MainWindow w;
    w.show();
    
    return a.exec();
}
