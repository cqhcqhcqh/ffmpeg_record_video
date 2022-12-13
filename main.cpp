#include "mainwindow.h"

#include <QApplication>
extern "C" {
   #include <libavformat/avformat.h>
   #include <libavdevice/avdevice.h>
}
#include <QDebug>
int main(int argc, char *argv[])
{
    avdevice_register_all();
    qDebug() << av_version_info();
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
