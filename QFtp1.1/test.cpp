#include "test.h"
test::test()
{

    connect(&ss,SIGNAL(G_tgetProgressVal(int)),this,SLOT(S_egetProgressVal(int)));

}
void test::sl()
{


    ss.SetInterface(this);
    ss.ftp_put("ftp://127.0.0.1","/test/333.txt","/test1/666.txt","dengtao","123456");
    //ss.ftp_get("ftp://192.168.5.115:21","/test1/test.txt","/test/ftp1227/test/test.txt","dengtao","123456");
    //ss.ftp_put("sftp://192.168.5.115","/test/test.txt","/tmp/1227/test/1227.txt","mysftp","dengtao1");
    //ss.ftp_get("sftp://192.168.5.115/tmp/","test11.txt","/test/sftp1227/test/test20.txt","mysftp","dengtao1");


}
void test::FtprevalueReport(int ss)
{

    if(ss==0)
    {
        fprintf(stderr,"ftp success");
    }
    else
    {
        fprintf(stderr,"ftp failed");
    }

}
void test::S_egetProgressVal(int tmpVal)
{
    qDebug()<<tmpVal;
}
