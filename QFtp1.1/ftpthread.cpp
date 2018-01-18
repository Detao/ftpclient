#include "ftpthread.h"
#include<stdlib.h>
ftpthread::ftpthread()
    :ftp_manage(NULL)
    ,_port(0)
{
    ftp_manage=new ftpmanage;
    ftp_manage->moveToThread(&workerThread);
    connect(this,
            SIGNAL(G_ftp_get(const QString&,const QString&,const QString&,const QString&,const QString&,qint16)),
            ftp_manage,
            SLOT(S_ftp_get(const QString&,const QString&,const QString&,const QString&,const QString&,qint16)));
    connect(this,
            SIGNAL(G_ftp_put(const QString&,const QString&,const QString&,const QString&,const QString&,qint16)),
            ftp_manage,
            SLOT(S_ftp_put(const QString&,const QString&,const QString&,const QString&,const QString&,qint16)));
    connect(this,
            SIGNAL(G_sftp_get(const QString&,const QString&,const QString&,const QString&,const QString&,qint16)),
            ftp_manage,
            SLOT(S_sftp_get(const QString&,const QString&,const QString&,const QString&,const QString&,qint16)));
    connect(this,
            SIGNAL(G_sftp_put(const QString&,const QString&,const QString&,const QString&,const QString&,qint16)),
            ftp_manage,
            SLOT(S_sftp_put(const QString&,const QString&,const QString&,const QString&,const QString&,qint16)));
    connect(ftp_manage, SIGNAL(resultReady(int)), this, SLOT(handleResults(int)));
    connect(ftp_manage,SIGNAL(G_mgetProgressVal(int)),this,SLOT(S_tgetProgressVal(int)));
    connect(&workerThread, &QThread::finished, this, &QObject::deleteLater);
    workerThread.start();

}
ftpthread::~ftpthread()
{
    workerThread.quit();
    workerThread.wait();
}
void ftpthread::SetInterface(FtpInterface *f)
{
    _interface=f;
}
void ftpthread::S_tgetProgressVal(int tmpVal)
{
    emit G_tgetProgressVal(tmpVal);
}
void ftpthread::ftp_get(const QString url,const QString remotefile,const QString localfile,
                        const QString username,const QString password,qint16 port)
{
    ParseUrl(url.toStdString());
    if(remotefile!=NULL)
    {
        _remotepath.append(remotefile);
    }
    if(username!=NULL)
    {
        _username=username;
    }
    if(password!=NULL)
    {
        _password=password;
    }
    if(port!=0)
    {
        _port=port;
    }
    if(_type==0)
    {
        if(_port==0)
            _port=21;
        emit G_ftp_get(_remotepath,localfile,_username,_password,_host,_port);
    }
    else if(_type==1)
    {
        if(_port==0)
            _port=22;

        emit G_sftp_get(_remotepath,localfile,_username,_password,_host,_port);
    }
    else
        qDebug()<<tr("url is incorrect %1").arg(url);
    clear();

}
void ftpthread::ftp_put(const QString url,const QString localfile,const QString remotefile,
                        const QString username,const QString password,qint16 port)
{
    ParseUrl(url.toStdString());
    if(remotefile!=NULL)
    {
        _remotepath.append(remotefile);
    }
    if(username!=NULL)
    {
        _username=username;
    }
    if(password!=NULL)
    {
        _password=password;
    }
    if(port!=0)
    {
        _port=port;
    }
    if(_type==0)
    {
        if(_port==0)
            _port=21;
        emit G_ftp_put(localfile,_remotepath,_username,_password,_host,_port);
    }
    else if(_type==1)
    {
        if(_port==0)
            _port=22;

        emit G_sftp_put(localfile,_remotepath,_username,_password,_host,_port);
    }
    else
        qDebug()<<tr("url is incorrect %1").arg(url);

    clear();

}
void ftpthread::handleResults(int a)
{
    _interface->FtprevalueReport(a);
}

void ftpthread::ParseUrl(std::string url)
{
    std::size_t found;
    if(url.compare(0,4,"sftp",4)==0)
    {
        _type=sftp_type;
        std::string ptr=url.substr(7);
        if((found=ptr.find_first_of("@"))!=std::string::npos)
        {
            _username=QString::fromStdString(url.substr(7,found));
            ptr=url.substr(found);
        }
        if((found=ptr.find_first_of(":"))!=std::string::npos)
        {
            _host=QString::fromStdString(ptr.substr(0,found));
            ptr=ptr.substr(found);
            if((found=ptr.find_first_of("/"))!=std::string::npos)
            {
                _port=atoi((ptr.substr(0,found)).c_str());
                _remotepath=QString::fromStdString(ptr.substr(found));
            }
            else
                _port=atoi(ptr.c_str());
        }
        else
        {
            if((found=ptr.find_first_of("/"))!=std::string::npos)
            {
                _host=QString::fromStdString(ptr.substr(0,found));
                _remotepath=QString::fromStdString(ptr.substr(found));
            }
            else
                _host=QString::fromStdString(ptr);

        }

    }
    else if(url.compare(0,3,"ftp",3)==0)
    {
        _type=ftp_type;
        std::string ptr=url.substr(6);
        if( (found=ptr.find_first_of("@"))!=std::string::npos)
        {
            std::size_t found_name;
            ptr=url.substr(6,found);
            if((found_name=ptr.find(":"))!=std::string::npos)
            {
                _username=QString::fromStdString(ptr.substr(0,found_name));
                _password=QString::fromStdString(ptr.substr(found_name+1));
            }
            else
            {
                _username=QString::fromStdString(ptr.substr(0,found_name));

            }
            ptr=url.substr(found+7);
        }
        if((found=ptr.find_first_of(":"))!=std::string::npos)
        {
            _host=QString::fromStdString(ptr.substr(0,found));
            ptr=ptr.substr(found);
            if((found=ptr.find_first_of("/"))!=std::string::npos)
            {
                _port=atoi((ptr.substr(0,found)).c_str());
                _remotepath=QString::fromStdString(ptr.substr(found+1));
            }
            else
                _port= atoi(ptr.c_str());
        }
        else
        {
            if(found=ptr.find_first_of("/")!=std::string::npos)
            {
                _host=QString::fromStdString(ptr.substr(0,found));
                _remotepath=QString::fromStdString(ptr.substr(found));
            }
            else
                _host=QString::fromStdString(ptr)  ;

        }
    }
    else
    {
        _type=sftp_error;
    }

}
void ftpthread::clear()
{
    _host.clear();
    _remotepath.clear();
    _username.clear();
    _password.clear();
    _port=0;
}
