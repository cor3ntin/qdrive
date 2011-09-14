CONFIG *= qt
QT *= core

INCLUDEPATH *= $$PWD/../include
DEPENDPATH  *= $$PWD

HEADERS += qdriveinfo.h \
           qdriveinfo_p.h \
           qdrivecontroller.h \
           qdrivecontroller_p.h \
           qdrive_global.h \
           qsystemerror_p.h

SOURCES += qdriveinfo.cpp \
           qdrivecontroller.cpp \
           qsystemerror.cpp

win32 {
    SOURCES += qdriveinfo_win.cpp \
               qdrivecontroller_win.cpp

    LIBS += -luserenv -lNetapi32 -lMpr -luser32
}

macx-* {
    SOURCES += qdriveinfo_mac.cpp \
               qdrivecontroller_mac.cpp

    LIBS += -framework CoreServices -framework DiskArbitration -framework IOKit
}

linux-*:!symbian {
    SOURCES += qdriveinfo_linux.cpp \
               qdrivecontroller_linux.cpp
    QT *= dbus
}

symbian {
    SOURCES += qdriveinfo_symbian.cpp \
               qdrivecontroller_symbian.cpp

    LIBS += -lefsrv
}
