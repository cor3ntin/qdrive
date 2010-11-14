QT -= gui
QT += testlib
TARGET = QDriveInfo

CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app

DEFINES += QDRIVEINFO_LIBRARY

#SOURCES += main.cpp
SOURCES += unittests.cpp

SOURCES += qdriveinfo.cpp
HEADERS += qdriveinfo.h \
           qdriveinfo_p.h

win32: {
    SOURCES += qdriveinfo_win.cpp
} else:unix {
    linux-*: {
        SOURCES += qdriveinfo_linux.cpp
    }
    macx-*: {
        SOURCES += qdriveinfo_mac.cpp
        LIBS *= -framework CoreServices -framework DiskArbitration -framework IOKit
    }
    symbian: {
        SOURCES += qdriveinfo_symbian.cpp
        LIBS *= -lefsrv
    }
}
