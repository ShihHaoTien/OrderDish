#include "mainwidget.h"
#include <QApplication>
#include <qdebug.h>


int main(int argc, char *argv[])
{
    qDebug("main start");
    QApplication a(argc, argv);
    mainWidget w;
    w.show();

    return a.exec();
}
