QT += core
QT += gui

CONFIG += c++11
QT += core network

TARGET = QFtp
CONFIG += console
CONFIG -= app_bundle
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TEMPLATE = app

SOURCES += main.cpp \
    qftp.cpp \
    sftp.cpp \
    ftp.cpp \
    ftpmanage.cpp \
    ftpthread.cpp \
    qurlinfo.cpp \
    test.cpp

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

HEADERS += \
    qftp.h \
    sftp.h \
    ftp.h \
    ftpmanage.h \
    ftpthread.h \
    qurlinfo.h \
    test.h

FORMS +=

DISTFILES +=


INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.

unix:!macx: LIBS += -L$$PWD/lib/ -lssl

INCLUDEPATH += $$PWD/lib
DEPENDPATH += $$PWD/lib

unix:!macx: PRE_TARGETDEPS += $$PWD/lib/libssl.a

unix:!macx: LIBS += -L$$PWD/lib/ -lssh2

INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include

unix:!macx: PRE_TARGETDEPS += $$PWD/lib/libssh2.a

unix:!macx: LIBS += -L$$PWD/lib/ -lcrypto

INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include

unix:!macx: PRE_TARGETDEPS += $$PWD/lib/libcrypto.a

