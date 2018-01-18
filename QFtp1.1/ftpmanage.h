#ifndef FTPMANAGE_H
#define FTPMANAGE_H
#include "ftp.h"
#include "sftp.h"
#include<QObject>

class ftpmanage: public QObject
{
    Q_OBJECT
public:
    ftpmanage(){}
    void abort();
signals:
    void resultReady(int);
    void G_abort();
    void G_mgetProgressVal(int);
public slots:

    void S_mgetProgressVal(int);
    void S_ftp_get(const QString&,const QString&,const QString& ,
                   const QString&,const QString& ,qint16);
    void S_ftp_put(const QString&,const QString&,const QString& ,
                   const QString&,const QString&,qint16);
    void S_sftp_get(const QString&,const QString&,const QString& ,
                    const QString&,const QString&,qint16);
    void S_sftp_put(const QString&,const QString&,const QString&,
                    const QString&,const QString&,qint16);
};

#endif // FTPMANAGE_H
