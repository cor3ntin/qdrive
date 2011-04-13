#-------------------------------------------------
#
# Project created by QtCreator 2010-06-20T14:31:59
#
#-------------------------------------------------

QT       += core gui

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

FORMS    += mainwindow.ui
