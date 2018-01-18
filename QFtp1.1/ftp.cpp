#include "ftp.h"
#include <QDebug>
#include <QIODevice>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
class ftp;
ftp::ftp(QObject *parent):
    QObject(parent),
    error(false),
    myFtp(NULL),
    serverfile_exist(false),
    command_id(0),
    remoteFileSize(0),
    m_File(0)
{
    myFtp = new QFtp();
    connect(myFtp,SIGNAL(listInfo(QUrlInfo)),this,SLOT(S_listInfo(QUrlInfo)));
    connect(myFtp,SIGNAL(commandFinished(int,bool)),
            this,SLOT(S_commandFinish(int,bool)));
    connect(myFtp,SIGNAL(dataTransferProgress(qint64,qint64)),
            SLOT(S_upDateProgress(qint64,qint64)));
}
ftp::~ftp()
{
    if(myFtp)
        delete myFtp;
    myFtp=nullptr;
}
void ftp::S_upDateProgress(qint64 _used, qint64 _total)
{
    int tmpVal = _used / (double)_total * 100;
    emit G_getProgressVal(tmpVal);
}
void ftp::S_abort()
{
    myFtp->abort();
}
int ftp::ftp_get( const QString& _remoteFile,const QString& _localFile ,
                  const QString& _userName,const QString& _passWd,
                  const QString& _host,qint16 _port)
{
    command_id=myFtp->connectToHost(_host,_port);
    get_result();
    if(error){
        return -1;
    }
    command_id=myFtp->login(_userName,_passWd);
    get_result();
    if(error){

        return -1;
    }
    dloadFile(_remoteFile,_localFile);
    if(error){
        return -1;
    }
    return 0;
}
int ftp::ftp_put(const QString& _localFile ,const QString& _remoteFile,
                 const QString& _userName,const QString& _passWd,
                 const QString& _host,qint16 _port)
{
    command_id=myFtp->connectToHost(_host,_port);
    get_result();
    if(error){
        return -1;
    }
    command_id=myFtp->login(_userName,_passWd);
    get_result();
    if(error){
        return -1;
    }
    uloadFile(_localFile,_remoteFile);
    if(error){
        return -1;
    }
    return 0;
}

//ftp服务提示信息
void ftp::S_commandFinish(int tmp, bool en)
{
    Q_UNUSED(tmp);
    if(command_id==tmp){
        if(myFtp->currentCommand() == QFtp::ConnectToHost){

            if(en){
                qDebug()<<myFtp->errorString();
                error=true;
            }
            bRet=true;
        }
        if (myFtp->currentCommand() == QFtp::Login){

            if(en){
                qDebug()<<myFtp->errorString();
                error=true;
            }
            bRet=true;

        }
        if(myFtp->currentCommand()==QFtp::List){
            if(en){
                qDebug()<<myFtp->errorString();
                error=true;
            }
            bRet=true;
        }
        if (myFtp->currentCommand() == QFtp::Get){
            if(en){
                qDebug()<<myFtp->errorString();
                error=true;
            }
            bRet=true;
        }else if(myFtp->currentCommand() == QFtp::Put){

            if(en){
                qDebug()<<myFtp->errorString();
                error=true;
            }
            bRet=true;
        }else if(myFtp->currentCommand()==QFtp::Rename){
            if(en){
                qDebug() << (tr("renamefailed：%1").arg(myFtp->errorString()));
                error=true;
            }
            bRet=true;
        }else if(myFtp->currentCommand()==QFtp::Remove){
            if(en){
                qDebug() << (tr("renamefailed：%1").arg(myFtp->errorString()));
                error=true;
            }
            bRet=true;
    }else if(myFtp->currentCommand() == QFtp::Close){
        bRet=true;
    }

}

}
//文件列表
void ftp::S_listInfo(QUrlInfo _urInfo)
{
    //qDebug() <<_urInfo.name()/*.toLatin1()*/<<" "<<_urInfo.size()<<" " <<_urInfo.owner();
    remoteFileSize=_urInfo.size();
    serverfile_exist=true;


}
bool  ftp::get_result()
{
    bRet=false;
    while (1){
        qApp->processEvents();   // 这里每100ms处理一次event，使slot函数能够被调用
        // bRet=semaphore.tryAcquire (1,1); // 等待信号100ms
        if(bRet)
            break;
    }
}
void ftp::dloadFile(const QString &_remoteFile,const QString &_localFile)
{
    QString local_log_name;
    QFileInfo fileinfo(_localFile);
    QString localfilepath=fileinfo.path();
    QDir dir(localfilepath);
    if(!dir.exists()){
        dir.mkpath(localfilepath);
    }
    serverfile_exist=false;
    command_id=myFtp->list(_remoteFile);
    get_result();

    if(error){
        return;
    }
    if(serverfile_exist){
        local_log_name=QString("%1.log").arg(_localFile);
        m_File=new QFile(local_log_name);
        if(!m_File->exists()){
            //qDebug()<< tr("文件%1的普通下载... ...").arg(_remoteFile);
            if(m_File->open(QIODevice::WriteOnly)){
                command_id=myFtp->get(_remoteFile,m_File);
                get_result();
                if(error){
                    m_File->close();
                    delete m_File;
                    m_File=NULL;
                    return;
                }
            }else{
                qDebug() << tr("localfile open failed").arg(_localFile);
                error=true;
            }
        }else{
            //qDebug() << tr("文件%1的续传下载... ...").arg(_remoteFile);
            if(m_File->open(QIODevice::Append)){
                if(remoteFileSize>m_File->size()){
                    myFtp->rawCommand(QString("REST %1").arg(m_File->size()));
                    myFtp->m_isConLoad = true;          //设置当前现在为续传
                    command_id=myFtp->get(_remoteFile,m_File);
                    get_result();
                    if(error){
                        m_File->close();
                        delete m_File;
                        m_File=NULL;
                        return;
                    }
                }else{
                    QFile::remove(local_log_name);
                    m_File->close();
                    delete m_File;
                    m_File = NULL;
                    dloadFile(_remoteFile,_localFile);
                    return;
                }
            }else{
                qDebug() << tr("localfile open failed").arg(_localFile);
                error=true;
            }
        }
        if(remoteFileSize==m_File->size())
        {
            if(QFile::exists(_localFile))
                QFile::remove(_localFile);
            QFile::rename(local_log_name,_localFile);
        }
        m_File->close();

    }else{
        qDebug()<<tr("remotefile does not exist %1").arg(_remoteFile);
        error=true;
    }
    delete m_File;
    m_File = NULL;
}
//上传文件（当isRese==true为续传上传）
void ftp::uloadFile(const QString &_localFile,const QString &_remoteFile)
{

    QString remote_log_name;
    remote_log_name=QString("%1.log").arg(_remoteFile);
    m_File = new QFile(_localFile);
    command_id=myFtp->list(remote_log_name);
    get_result();
    if(error){
        return;
    }
    if(m_File->open(QIODevice::ReadOnly)){
        if(!serverfile_exist){
            //qDebug() << tr("文件%1的普通上传... ...").arg(_localFile);
            command_id=myFtp->put(m_File,remote_log_name);
            get_result();
            if(error){
                m_File->close();
                delete m_File;
                m_File=NULL;
                return;
            }
        }else{
            //qDebug() << tr("文件%1的续传... ...").arg(_localFile);
            if(remoteFileSize<=m_File->size()){
                //m_File->seek(remoteFileSize);
                command_id=myFtp->conPut(m_File,remote_log_name);
                get_result();
                if(error){
                    m_File->close();
                    delete m_File;
                    m_File=NULL;
                    return;
                }
            }else{
                command_id=myFtp->remove(remote_log_name);
                get_result();
                if(error){
                    m_File->close();
                    delete m_File;
                    m_File=NULL;
                    return;
                }
                command_id=myFtp->put(m_File,remote_log_name);
                get_result();
                if(error){
                    m_File->close();
                    delete m_File;
                    m_File=NULL;
                    return;
                }
            }
        }
        command_id=myFtp->list(remote_log_name);
        get_result();
        if(error){
            m_File->close();
            delete m_File;
            m_File=NULL;
            return;
        }
        if(remoteFileSize==m_File->size()){
            command_id=myFtp->rename(remote_log_name,_remoteFile);
            get_result();
            if(error){
                m_File->close();
                delete m_File;
                m_File=NULL;
                return;
            }
        }
        m_File->close();
    }else{
        qDebug() << tr("localfile open failed").arg(_localFile);
        error=true;
    }

    delete m_File;
    m_File = NULL;
}


