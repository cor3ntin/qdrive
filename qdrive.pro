# -------------------------------------------------
# Project created by QtCreator 2010-08-27T18:28:12
# -------------------------------------------------
QT -= gui
TARGET = QDriveInfo

CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app

SOURCES += main.cpp \
    qdrive.cpp
HEADERS += qdrive.h \
    qdrive_p.h

win32: {
    HEADERS += qdrive_win_p.h
    SOURCES += qdrive_win.cpp
}

unix: {
    linux-*: {
       HEADERS += qdrive_linux_p.h
       SOURCES += qdrive_linux.cpp
    }
    mac: {
       HEADERS += qdrive_mac_p.h
       SOURCES += qdrive_mac.cpp
       LIBS += -framework CoreServices -framework DiskArbitration -framework IOKit
    }
}
