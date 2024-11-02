#include "mainwindow.h"
#include <QApplication>
#include<qtextcodec.h>

int main(int argc, char *argv[])
{
    //QTextCodec* codec = QTextCodec::codecForName("UTF-8");
    //QTextCodec::setCodecForLocale(codec);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    w.setWindowTitle("MY_TFTP");
    return a.exec();
}
