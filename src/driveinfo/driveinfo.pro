TARGET = QtDriveInfo
QT = core core-private

CONFIG -= rtti exceptions
DEFINES *= QT_NO_CAST_FROM_BYTEARRAY QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII

win32-msvc*|win32-icc:QMAKE_LFLAGS += /BASE:0x66000000
solaris-cc*:QMAKE_CXXFLAGS_RELEASE -= -O2

load(qt_module)

HEADERS += qtdriveinfoglobal.h
HEADERS += qdriveinfo.h \
           qdriveinfo_p.h \
           qdrivecontroller.h \
           qdrivecontroller_p.h
SOURCES += qdriveinfo.cpp \
           qdrivecontroller.cpp

win* {
    SOURCES += qdriveinfo_win.cpp \
               qdrivecontroller_win.cpp

    LIBS_PRIVATE += -luserenv -lNetapi32 -lMpr -luser32 -lWinmm
}

macx-* {
    SOURCES += qdriveinfo_mac.cpp \
               qdrivecontroller_mac.cpp

    LIBS_PRIVATE += -framework CoreServices -framework DiskArbitration -framework IOKit
}

linux-*:!symbian {
    SOURCES += qdriveinfo_linux.cpp \
               qdrivecontroller_linux.cpp
    QT *= dbus
}

#symbian {
#    SOURCES += qdriveinfo_symbian.cpp \
#               qdrivecontroller_symbian.cpp
#
#    LIBS_PRIVATE += -lefsrv
#}
