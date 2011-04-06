TARGET    = qdriveinfo
win32:!wince*:DLLDESTDIR = $$OUT_PWD/../bin

CONFIG    *= qt warn_on
QT        = core
TEMPLATE  = lib

include( $$PWD/../build_version.pri )
include( $$PWD/qdriveinfo.pri )
include( $$PWD/../src/src.pri )

# hardcoded:
mac: LIBS += -dynamiclib -Wl,-install_name,$$OUT_PWD/libqdriveinfo_debug.1.dylib

!contains(CONFIG, staticlib) {
    win32:DEFINES *= QDRIVEINFO_MAKEDLL
}

symbian: {
 CONFIG += staticlib
}

