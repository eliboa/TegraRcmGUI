#include "tegrarcmgui.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TegraRcmGUI w;
    w.show();
    return a.exec();
}
