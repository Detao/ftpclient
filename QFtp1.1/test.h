#ifndef TEST_H
#define TEST_H
#include"ftpthread.h"
class test :public QObject,public FtpInterface
{
    Q_OBJECT
    ftpthread ss;
public:
    test();
    void sl();

    void FtprevalueReport(int ss);

public slots:
    void S_egetProgressVal(int);

};
#endif // TEST_H
