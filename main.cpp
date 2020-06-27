#include "tegrarcmgui.h"

#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);
    a.setStyleSheet("QMainWindow{ border-radius: 20px; }");
    QTranslator appTranslator;
    //appTranslator.load(QLocale(), QLatin1String("tegrarcmgui"), QLatin1String("_"), QLatin1String(":/i18n"));
    appTranslator.load("tegrarcmgui_fr", "languages");
    a.installTranslator(&appTranslator);
    TegraRcmGUI w;
    w.setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    w.show();
    return a.exec();
}
