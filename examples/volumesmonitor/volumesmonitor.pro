TARGET = volumesmonitor
QT = core
CONFIG += console

include( $$PWD/../example.pri )

HEADERS += monitor.h

SOURCES += main.cpp \
        monitor.cpp
