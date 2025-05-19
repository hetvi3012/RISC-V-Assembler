#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    w.setWindowTitle("RiscV Simulator");

    QIcon appIcon(":/images/icon.png");
    w.setWindowIcon(appIcon);

    w.show();
    return a.exec();
}
