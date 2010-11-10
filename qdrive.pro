# -------------------------------------------------
# Project created by QtCreator 2010-08-27T18:28:12
# -------------------------------------------------
QT -= gui
TARGET = QDriveInfo

CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app

DEFINES += QDRIVEINFO_LIBRARY

SOURCES += main.cpp

SOURCES += qdriveinfo.cpp
HEADERS += qdriveinfo.h \
           qdriveinfo_p.h

win32: SOURCES += qdriveinfo_win.cpp
unix: linux-*: SOURCES += qdriveinfo_linux.cpp
mac: {
   SOURCES += qdriveinfo_mac.cpp
   LIBS += -framework CoreServices -framework DiskArbitration -framework IOKit
}
