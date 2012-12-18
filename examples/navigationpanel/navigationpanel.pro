QT       += core gui widgets

TARGET = NavigationPanel
TEMPLATE = app

include( $$PWD/../example.pri )

SOURCES += main.cpp\
        mainwindow.cpp \
        navigationpanel.cpp \
    navigationmodel.cpp

HEADERS  += mainwindow.h \
        navigationpanel.h \
    navigationmodel.h \
    navigationmodel_p.h
