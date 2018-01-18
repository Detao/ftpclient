#ifndef FTPTHREAD_H
#define FTPTHREAD_H
#include"ftpmanage.h"
#include<QThread>
enum type{
    ftp_type,
    sftp_type,
    sftp_error
};
class FtpInterface{
public:
    virtual void FtprevalueReport(int revalue)=0;
    virtual void FtpErorrvalueReport(std::string *reErorrValue){}
};
class ftpthread :public QObject
{
    Q_OBJECT
    QThread workerThread;
public:
    ftpthread();
    ~ftpthread();
    void ftp_get(QString url,QString remotefile,QString localfile,
                 QString username,QString password,
                 qint16 port=0);
    void ftp_put(QString url,QString localfile,QString remotefile,
                 QString username,QString password,
                 qint16 port=0);
    void SetInterface(FtpInterface *f);
    void abort();
signals:
    void G_ftp_get(const QString&,const QString&,const QString&,
                   const QString&,const QString&,qint16);
    void G_ftp_put(const QString&,const QString&,const QString&,
                   const QString&,const QString&,qint16);
    void G_sftp_get(const QString&,const QString&,const QString&,
                    const QString&,const QString&,qint16);
    void G_sftp_put(const QString&,const QString&,const QString&,
                    const QString&,const QString&,qint16);
    void G_abort();
    void G_tgetProgressVal(int);
public slots:
    void S_tgetProgressVal(int);
    void handleResults(int);
private:
    void ParseUrl(std::string url);
    void clear();
    ftpmanage    *ftp_manage;
    FtpInterface *_interface;
    type          _type;
    QString       _host;
    QString       _remotepath;
    QString       _username;
    QString       _password;
    qint16        _port;
};

#endif // FTPTHREAD_H
