#include "ftpmanage.h"
void ftpmanage::S_ftp_get(const QString& remotefile, const QString& localfile,
                          const QString& username,const QString& password,
                          const QString& host, qint16 port)
{

    ftp a;
    connect(&a,SIGNAL(G_getProgressVal(int)),
            this,SLOT(S_mgetProgressVal(int)));
    if(a.ftp_get(remotefile,localfile,username,password,host ,port)==0)
        emit resultReady(0);
    else
        emit resultReady(-1);

}
void ftpmanage::S_mgetProgressVal(int tmpVal)
{
   emit G_mgetProgressVal(tmpVal);
}
void ftpmanage::abort()
{
  //emit G_abort();
}
void ftpmanage::S_ftp_put(const QString& localfile,const QString& remotefile,
                            const QString& username,const QString& password,
                            const QString& host, qint16 port)
{
    ftp a;
    connect(&a,SIGNAL(G_getProgressVal(int)),
            SLOT(S_mgetProgressVal(int)));
    if(a.ftp_put(localfile,remotefile,username,password,host ,port)==0)
        emit resultReady(0);
    else
        emit resultReady(-1);
}
void ftpmanage::S_sftp_get(const QString& remotefile, const QString& localfile,
                           const QString& username,const QString& password,
                           const QString& host, qint16 port)
{
    sftp a;
    connect(&a,SIGNAL(G_getProgressVal(int)),
            SLOT(S_mgetProgressVal(int)));
    if(a.sftp_get(remotefile,localfile,username,password,host ,port)==0)
        emit resultReady(0);
    else
        emit resultReady(-1);
}
void ftpmanage::S_sftp_put(const QString& localfile,const QString& remotefile,
                            const QString& username,const QString& password,
                            const QString& host, qint16 port)
{
    sftp a;
    connect(&a,SIGNAL(G_getProgressVal(int)),
            SLOT(S_mgetProgressVal(int)));
    if(a.sftp_put(localfile,remotefile,username,password,host ,port)==0)
        emit resultReady(0);
    else
        emit resultReady(-1);
}
