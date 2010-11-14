TEMPLATE = app
CONFIG += qt warn_on console depend_includepath
CONFIG -= app_bundle

qtAddLibrary(QtTest)

DESTDIR = $$OUT_PWD/../../bin

include( $$PWD/../build_version.pri )
include( $$PWD/../lib/qdriveinfo.pri )
LIBS *= $$QDRIVEINFO_LIBS -L$$OUT_PWD/../../lib
