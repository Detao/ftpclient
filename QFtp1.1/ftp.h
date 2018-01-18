#ifndef FTPMANAGER_H
#define FTPMANAGER_H

#include <QObject>
#include "qftp.h"
#include <QFile>
class ftp : public QObject
{
     Q_OBJECT
public:

    explicit ftp( QObject *parent = 0);
    ~ftp();
    QString errorString() const;

    int ftp_get(const QString&,const QString&,const QString&,
                   const QString&,const QString&,qint16);
    int ftp_put(const QString&,const QString&,const QString&,
                   const QString&,const QString&,qint16);
signals:
    void G_getProgressVal(int);
public slots:
    void S_commandFinish(int,bool);
    void S_listInfo(QUrlInfo s);
    void S_abort();
    void S_upDateProgress(qint64,qint64);

private:
    bool get_result();
    void dloadFile(const QString& _remoteFile,const QString& _localFile);
    void uloadFile(const QString& _localFile,const QString& _remoteFile);


    bool            error;
    QFtp            *myFtp;
    bool            serverfile_exist;
    int             command_id;
    long long       remoteFileSize;
    QFile           *m_File;
    bool            bRet;
};

#endif // FTPMANAGER_H
