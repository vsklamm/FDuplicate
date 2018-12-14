#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    qRegisterMetaType<std::set<QString>>("std::set<QString>");
    qRegisterMetaType<QVector<QVector<extended_file_info>>>("QVector<QVector<extended_file_info>>");

    QApplication a(argc, argv);

    // a.setStyle("Plastique");

    MainWindow w;
    w.show();

    return a.exec();
}
