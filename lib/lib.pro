TARGET    = qdriveinfo
DESTDIR   = $$OUT_PWD
win32:!wince*:DLLDESTDIR = $$OUT_PWD/../bin

CONFIG    *= qt warn_on
QT        = core
TEMPLATE  = lib

include( $$PWD/../build_version.pri )
include( $$PWD/qdriveinfo.pri )
include( $$PWD/../src/src.pri )

!contains(CONFIG, staticlib) {
    win32:DEFINES *= QDRIVEINFO_MAKEDLL
}
