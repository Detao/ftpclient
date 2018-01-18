#include "sftp.h"
#include<sys/poll.h>
#include <QTime>
#include <QDataStream>
#include <assert.h>
#include <QFileInfo>
#include <QDir>
sftp::sftp()
    :m_session(NULL)
    ,m_sftp(NULL)
    ,m_buf(NULL)
    ,m_buflen(1024*16)
    ,m_finish(0)
    ,m_errmsg(NULL)
    ,m_errmsg_buflen(1024)
{
    m_buf=new char[m_buflen];
    assert(m_buf);
    m_errmsg=new char[m_errmsg_buflen];
    assert(m_errmsg);
    connect(this,SIGNAL(dataTransferProgress(qint64,qint64)),
            SLOT(S_upDateProgress(qint64,qint64)));
}

sftp::~sftp()
{
    delete []m_buf;
    m_buf=NULL;
}
void sftp::abort()
{
    sftp_disconnect();
}
void sftp::S_upDateProgress(qint64 _used, qint64 _total)
{
    int tmpVal = _used / (double)_total * 100;
    emit G_getProgressVal(tmpVal);
}
int sftp::sftp_get(const QString& remotefile,const QString& localfile,
                   const QString& username ,const QString& password,const QString& host,qint16 port)
{
    if(sftp_connect(host,username,password,port)==0){
        if(sftp_get(remotefile,localfile)==0){
            sftp_disconnect();
            return 0;
        }
    }

    sftp_disconnect();
    return -1;
}
int sftp::sftp_put(const QString& remotefile,const QString& localfile,
                   const QString& username ,const QString& password,const QString& host,qint16 port)
{

    if(sftp_connect(host,username,password,port)==0){
        if(sftp_put(remotefile,localfile)==0){
            sftp_disconnect();
            return 0;
        }
    }
    sftp_disconnect();
    return -1;

}
void sftp::sftp_disconnect()
{
    if(m_session){
        libssh2_session_disconnect(m_session,"Normal shutdown");
        libssh2_session_free(m_session);
        m_session=NULL;
    }
    if(m_sock){
        m_sock->close();
        delete m_sock;
        m_sock=NULL;
    }
}

int sftp::sftp_connect(const QString& host,  const QString& username,const QString& password,int port)
{
    QByteArray _username = username.toLatin1(); // must
    QByteArray _password = password.toLatin1(); // must
    int        rc;//lingshi
    char       *userauthlist;
    m_sock=new QTcpSocket;
    m_sock->connectToHost(host,port);
    if(!m_sock->waitForConnected(5000)){
        qDebug("Error connecting to host %s",host.toLocal8Bit().constData());
        return -1;
    }
    m_session = libssh2_session_init();

    if(!m_session){
        get_lib_error();
        return -1;
    }
    rc = libssh2_session_handshake(m_session, m_sock->socketDescriptor());
    if(rc) {
        get_lib_error();
        qDebug()<<tr("Failure establishing SSH session %1").arg(m_errmsg);
        return -1;
    }
    libssh2_session_set_blocking(m_session, 0);
    /* check what authentication methods are available */
    userauthlist=NULL;
    while((userauthlist = libssh2_userauth_list(m_session, _username.data(), username.size()))==NULL
          &&waitsocket()>0);
    if(NULL==userauthlist||strstr(userauthlist,"password")==NULL){
        get_lib_error();
        sftp_disconnect();
        qDebug()<<tr("Authentication not by password %1").arg(m_errmsg);
        return -1;
    }
    while((rc=libssh2_userauth_password(m_session,_username.data(),_password.data()))==LIBSSH2_ERROR_EAGAIN
          &&waitsocket()>0);

    if(rc){
        get_lib_error();
        sftp_disconnect();
        qDebug()<<tr("Authentication by password failed %1").arg(m_errmsg);
        return -1;
    }
    return 0;
}
int sftp::sftp_get(const QString& remotefile,const QString& localfile)
{
    LIBSSH2_SFTP_ATTRIBUTES attrs;
    LIBSSH2_SFTP_HANDLE *sftp_handle;
    int rc=0;//lingshi
    long long remotefilesize=0;
    int read_size=0;

    QString localfile_log=QString("%1.log").arg(localfile);
    m_File=new QFile(localfile_log);
    QFileInfo fileinfo(localfile);
    QString localfilepath=fileinfo.path();
    QDir dir(localfilepath);
    if(!dir.exists()){
        dir.mkpath(localfilepath);
    }
    QByteArray _remotefile = remotefile.toLatin1(); // must
    while((m_sftp = libssh2_sftp_init(m_session))==NULL
          &&libssh2_session_last_error(m_session, NULL, NULL, 0)==LIBSSH2_ERROR_EAGAIN
          &&waitsocket()>0);

    if (!m_sftp) {
        get_lib_error();
        qDebug()<<tr("Unable to init SFTP session %1").arg(m_errmsg);
        return -1;
    }
    while((sftp_handle =
           libssh2_sftp_open(m_sftp, _remotefile.data(), LIBSSH2_FXF_WRITE|LIBSSH2_FXF_READ,
                             LIBSSH2_SFTP_S_IRUSR|LIBSSH2_SFTP_S_IWUSR|
                             LIBSSH2_SFTP_S_IRGRP|LIBSSH2_SFTP_S_IROTH))==NULL
          &&libssh2_session_last_errno(m_session)==LIBSSH2_ERROR_EAGAIN
          &&waitsocket()>0);

    if (!sftp_handle) {
        get_lib_error();
        qDebug()<<tr("Unable to open file with SFTP %1").arg(m_errmsg);
        return -1;
    }
    while((rc=libssh2_sftp_fstat_ex(sftp_handle, &attrs, 0))==LIBSSH2_ERROR_EAGAIN
          &&waitsocket()>0);
    if(rc<0){
        get_lib_error();
        qDebug()<<tr("libssh2_sftp_fstat_ex failed %1").arg(m_errmsg);
        return -1;
    }
    remotefilesize=attrs.filesize;
    if(m_File->exists()){
        int file_lenth=m_File->size();

        if(file_lenth<=remotefilesize){
            if(!m_File->open(QIODevice::Append)){
                qDebug()<<tr("localfile open failed %1").arg(localfile);
                return -1;
            }
            m_finish+=file_lenth;
            libssh2_sftp_seek64(sftp_handle, file_lenth);
            if (!sftp_handle) {
                get_lib_error();
                qDebug()<<tr("libssh2_sftp_seek64 failed %1").arg(m_errmsg);
                return -1;
            }
            do{
                while((read_size= libssh2_sftp_read(sftp_handle, m_buf, m_buflen))==LIBSSH2_ERROR_EAGAIN
                      &&waitsocket());
                if (read_size > 0){
                    int write_size=0, fn=read_size;
                    char *ptr=m_buf;
                    do{
                        write_size=m_File->write(ptr,fn);
                        if(write_size==-1){
                            return -1;
                        }
                        fn-=write_size;
                        ptr+write_size;
                    }while(fn);

                }else{
                    break;
                }
                m_finish+=read_size;
                emit dataTransferProgress(m_finish,remotefilesize);
            } while (1);
        }else{
            m_File->remove();
            m_File->close();
            delete m_File;
            m_File=NULL;
            libssh2_sftp_close(sftp_handle);
            libssh2_sftp_shutdown(m_sftp);
            if(sftp_get(remotefile,localfile)==0)
                return 0;
            return -1;
        }
    }else{
        if(!m_File->open(QIODevice::WriteOnly)){
            qDebug()<<tr("localfile open failed %1").arg(localfile);
            return -1;
        }
        do{
            /* loop until we fail */

            while((read_size = libssh2_sftp_read(sftp_handle, m_buf, m_buflen))==LIBSSH2_ERROR_EAGAIN
                  &&waitsocket());

            if(read_size > 0){
                int write_size=0, fn=read_size;
                char *ptr=m_buf;
                do{
                    write_size=m_File->write(ptr,fn);
                    if(write_size==-1){
                        return -1;
                    }
                    fn-=write_size;
                    ptr+write_size;
                }while(fn);
            }else{
                break;
            }
            m_finish+=read_size;
            emit dataTransferProgress(m_finish,remotefilesize);
        }while (1);
    }
    if(m_File->size()==remotefilesize){
        if(!m_File->rename(localfile)){
            QFile::remove(localfile);
            m_File->rename(localfile);
        }
        m_File->close();
        delete m_File;
        m_File=NULL;
        libssh2_sftp_close(sftp_handle);
        libssh2_sftp_shutdown(m_sftp);
        return 0;
    }

    m_File->close();
    delete m_File;
    m_File=NULL;
    libssh2_sftp_close(sftp_handle);
    libssh2_sftp_shutdown(m_sftp);
    return -1;
}
int sftp::sftp_put(const QString& localfile,const QString& remotefile)
{
    LIBSSH2_SFTP_ATTRIBUTES attrs;
    LIBSSH2_SFTP_HANDLE *sftp_handle;
    long long remotefilesize=0;
    int rc=0;
    int write_size=0;
    QString remotefile_log=QString("%1.log").arg(remotefile);

    QByteArray _remotefile = remotefile.toLatin1(); // must
    QByteArray _remotefile_log = remotefile_log.toLatin1(); // must
    m_File=new QFile(localfile);
    while((m_sftp = libssh2_sftp_init(m_session))==NULL
          &&libssh2_session_last_error(m_session, NULL, NULL, 0)==LIBSSH2_ERROR_EAGAIN
          &&waitsocket()>0);
    if (!m_sftp){
        get_lib_error();
        qDebug()<<tr("Unable to init SFTP session %1").arg(m_errmsg);
        return -1;
    }
    std::string remotefilepath=_remotefile.toStdString();
    std::size_t foundnum;
    std::string path;
    while((foundnum=remotefilepath.find_first_of("/"))!=std::string::npos)
    {
        //mkdir
        path.append(remotefilepath.substr(0,foundnum));
        path.append("/");
        if(!path.empty()){
            while((sftp_handle =
                   libssh2_sftp_opendir(m_sftp, path.data()))==NULL
                  &&libssh2_session_last_errno(m_session)==LIBSSH2_ERROR_EAGAIN
                  &&waitsocket()>0);
            if (!sftp_handle) {
                while((rc = libssh2_sftp_mkdir(m_sftp, path.data(),
                                               LIBSSH2_SFTP_S_IRWXU|LIBSSH2_SFTP_S_IRGRP|
                                               LIBSSH2_SFTP_S_IXGRP|LIBSSH2_SFTP_S_IROTH|
                                               LIBSSH2_SFTP_S_IXOTH))==LIBSSH2_ERROR_EAGAIN
                      &&waitsocket()>0);

                if(rc)
                    fprintf(stderr, "libssh2_sftp_mkdir failed: %d\n", rc);
            }
        }
        remotefilepath=remotefilepath.substr(foundnum+1);
    }

    while((sftp_handle =
           libssh2_sftp_open(m_sftp, _remotefile_log.data(), LIBSSH2_FXF_WRITE|LIBSSH2_FXF_CREAT,
                             LIBSSH2_SFTP_S_IRUSR|LIBSSH2_SFTP_S_IWUSR|
                             LIBSSH2_SFTP_S_IRGRP|LIBSSH2_SFTP_S_IROTH|
                             LIBSSH2_SFTP_RENAME_OVERWRITE))==NULL
          &&libssh2_session_last_errno(m_session)==LIBSSH2_ERROR_EAGAIN
          &&waitsocket()>0);

    if (!sftp_handle) {
        get_lib_error();
        qDebug()<<tr("Unable to open file with SFTP %1").arg(m_errmsg);
        return -1;
        //goto shutdown;
    }

    while((rc=libssh2_sftp_fstat_ex(sftp_handle, &attrs, 0))==LIBSSH2_ERROR_EAGAIN
          &&waitsocket()>0);

    if(rc<0){
        get_lib_error();
        qDebug()<<tr("libssh2_sftp_fstat_ex failed %1").arg(m_errmsg);
        return -1;
    }
    remotefilesize=attrs.filesize;

    if(m_File->open(QIODevice::ReadOnly)){
        if(remotefilesize!=0){
            //conput
            if(remotefilesize<=m_File->size()){
                m_finish+=remotefilesize;
                m_File->seek(remotefilesize);
                libssh2_sftp_seek64(sftp_handle, remotefilesize);
                if (!sftp_handle) {
                    m_File->close();
                    delete m_File;
                    m_File=NULL;
                    get_lib_error();
                    qDebug()<<tr("libssh2_sftp_seek64 %1").arg(m_errmsg);
                    return -1;
                }
                do{
                    int nread=m_File->read(m_buf,m_buflen);
                    if (nread <= 0) {
                        /* end of file */
                        break;
                    }
                    char *ptr=m_buf;
                    do{
                        /* write data in a loop until we block */
                        while((write_size = libssh2_sftp_write(sftp_handle, ptr, nread))==LIBSSH2_ERROR_EAGAIN
                              &&waitsocket()>0);

                        if(write_size < 0){
                            get_lib_error();
                            qDebug()<<tr("SFTP write file failed %1").arg(m_errmsg);
                            break;
                        }

                        ptr += write_size;
                        nread -= write_size;
                        m_finish+=write_size;
                        emit dataTransferProgress(m_finish,m_File->size());
                    } while (nread);
                }while (write_size > 0);
            }else{
                libssh2_sftp_close(sftp_handle);
                while((rc=libssh2_sftp_unlink(m_sftp,
                                              _remotefile_log.data()))== LIBSSH2_ERROR_EAGAIN
                      &&waitsocket()>0);
                if(rc){
                    get_lib_error();
                    qDebug()<<tr("libssh2_sftp_unlink failed %1").arg(m_errmsg);
                }
                m_File->close();
                delete m_File;
                m_File=NULL;
                libssh2_sftp_shutdown(m_sftp);
                if(sftp_put(localfile,remotefile)==0)
                    return 0;
                return -1;
            }

        }else if(remotefilesize==0){
            //put
            do{
                int nread=m_File->read(m_buf,m_buflen);
                if (nread <= 0){
                    /* end of file */
                    break;
                }
                char *ptr=m_buf;
                do{
                    /* write data in a loop until we block */
                    while((write_size = libssh2_sftp_write(sftp_handle, ptr, nread))==LIBSSH2_ERROR_EAGAIN
                          &&waitsocket()>0);
                    if(write_size < 0){
                        get_lib_error();
                        qDebug()<<tr("SFTP write file failed %1").arg(m_errmsg);
                        break;
                    }
                    ptr += write_size;
                    nread -= write_size;
                    m_finish+=write_size;
                    emit dataTransferProgress(m_finish,m_File->size());
                }while (nread);

            }while (write_size > 0);
        }else{
            libssh2_sftp_close(sftp_handle);
            libssh2_sftp_shutdown(m_sftp);
            return -1;
        }

        while((rc=libssh2_sftp_fstat_ex(sftp_handle, &attrs, 0))==LIBSSH2_ERROR_EAGAIN
              &&waitsocket()>0);

        if(rc){
            get_lib_error();
            qDebug()<<tr("libssh2_sftp_fstat_ex failed %1").arg(m_errmsg);
            return -1;
        }
        remotefilesize=attrs.filesize;
        if(m_File->size()==remotefilesize){
            while((rc=libssh2_sftp_unlink(m_sftp,
                                          _remotefile.data()))== LIBSSH2_ERROR_EAGAIN
                  &&waitsocket()>0);
            if(rc){
                get_lib_error();
                qDebug()<<tr("libssh2_sftp_unlink failed %1").arg(m_errmsg);
            }
            while((rc=libssh2_sftp_rename(m_sftp,_remotefile_log.data(),
                                          _remotefile.data()))== LIBSSH2_ERROR_EAGAIN
                  &&waitsocket()>0);
            if(rc){
                qDebug()<<tr("libssh2_sftp_rename %1").arg(m_errmsg);
            }
            m_File->close();
            delete m_File;
            m_File=NULL;
            libssh2_sftp_close(sftp_handle);
            libssh2_sftp_shutdown(m_sftp);
            return 0;
        }
        m_File->close();
        delete m_File;
        m_File=NULL;
        libssh2_sftp_close(sftp_handle);
        libssh2_sftp_shutdown(m_sftp);
        return -1;
    }else{
        qDebug()<<tr("localfile open failed %1").arg(localfile);
    }
}
int sftp::waitsocket()
{
    struct timeval timeout;
    int rc;
    fd_set fd;
    fd_set *writefd = NULL;
    fd_set *readfd = NULL;
    int dir;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    FD_ZERO(&fd);
    FD_SET(m_sock->socketDescriptor(), &fd);
    /* now make sure we wait in the correct direction */
    dir = libssh2_session_block_directions(m_session);
    if(dir & LIBSSH2_SESSION_BLOCK_INBOUND)
        readfd = &fd;
    if(dir & LIBSSH2_SESSION_BLOCK_OUTBOUND)
        writefd = &fd;
    rc = select(m_sock->socketDescriptor() + 1, readfd, writefd, NULL, &timeout);

    return rc;
}
