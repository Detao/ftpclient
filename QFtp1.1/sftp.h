#ifndef SFTP_H
#define SFTP_H
#include <QTcpServer>
#include <QTcpSocket>
#include <libssh2.h>
#include <libssh2_sftp.h>
#include <QObject>
#include <QFile>
class sftp : public QObject
{
    Q_OBJECT
public:
    sftp();
    ~sftp();
    int sftp_get(const QString& remotefile,const QString& localfile,
                 const QString& username ,const QString& password,const QString& host,qint16 port=22);
    int sftp_put(const QString& localfile,const QString& remotefile,
                const QString& username ,const QString& password,const QString& host,qint16 port=22);

signals:
    void dataTransferProgress(qint64,qint64);
    void G_getProgressVal(int);

public slots:
    void abort();
    void S_upDateProgress(qint64,qint64);

private:
    void sftp_disconnect();
    int sftp_connect(const QString& host, const QString& username ,const QString& password,int port=22);
    void get_lib_error()
    {
        m_errno = libssh2_session_last_errno(m_session);
        char* errmsg = NULL;
        int errmsg_len = 0;
        libssh2_session_last_error(m_session, &errmsg, &errmsg_len, 0);
        if (errmsg_len>0 && errmsg)
        {
            strncpy(m_errmsg, errmsg, m_errmsg_buflen);
        }
    }
    int sftp_get(const QString& remotefile,const QString& localfile);
    int sftp_put(const QString& localfile,const QString& remotefile);
    int waitsocket();
    QTcpSocket          *m_sock;
    LIBSSH2_SESSION     *m_session;
    LIBSSH2_SFTP        *m_sftp;


    char                *m_buf;
    int                  m_buflen;
    int                  m_errno;
    char                *m_errmsg;
    int                  m_errmsg_buflen;
    long long            m_finish;
    QFile                *m_File;

};

#endif // SFTP_H
