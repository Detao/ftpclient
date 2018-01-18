#include"ftp.h"
#include <QDebug>
#include <QApplication>
#include"qftp.h"
#include<QFile>
#include<unistd.h>
#include "sftp.h"

#include"ftpmanage.h"
#include <QCoreApplication>
#include <QObject>
#include <QStringList>

#include <cstdlib>
#include <iostream>
#include <qftp.h>
#include "test.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    test ll;
    ll.sl();
    return a.exec();
}


