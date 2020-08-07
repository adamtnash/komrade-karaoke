#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    qRegisterMetaType<ChannelPair>();
    qRegisterMetaTypeStreamOperators<ChannelPair>("ChannelPair");

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
