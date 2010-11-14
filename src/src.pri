CONFIG *= qt
QT *= core

INCLUDEPATH *= $$PWD/../include
DEPENDPATH  *= $$PWD

HEADERS += qdriveinfo.h \
           qdriveinfo_p.h

SOURCES += qdriveinfo.cpp

win32 {
    SOURCES += qdriveinfo_win.cpp
} else: unix {
    linux-* {
        SOURCES += qdriveinfo_linux.cpp
    }
    macx-* {
        SOURCES += qdriveinfo_mac.cpp
        LIBS += -framework CoreServices -framework DiskArbitration -framework IOKit
    }
    symbian {
        SOURCES += qdriveinfo_symbian.cpp
        LIBS += -lefsrv
    }
}
