CONFIG *= qt
QT *= core

INCLUDEPATH *= $$PWD/../include
DEPENDPATH  *= $$PWD

HEADERS += qdriveinfo.h \
           qdriveinfo_p.h \
           qdrivecontroller.h \
           qdrivecontroller_p.h \
           qdrive_global.h

SOURCES += qdriveinfo.cpp \
           qdrivecontroller.cpp

win32 {
    SOURCES += qdriveinfo_win.cpp #\
               #qdrivecontroller_win.cpp
    LIBS += -luserenv
} else: unix {
    linux-* {
        SOURCES += qdriveinfo_linux.cpp \
                qdrivecontroller_linux.cpp
    }
    macx-* {
        SOURCES += qdriveinfo_mac.cpp \
                qdrivecontroller_mac.cpp
        LIBS += -framework CoreServices -framework DiskArbitration -framework IOKit
    }
    symbian {
        SOURCES += qdriveinfo_symbian.cpp \
                qdrivecontroller_symbian.cpp
        LIBS *= -lefsrv
        LIBS *= $$QMAKE_LIBS_CORE
    }
}
