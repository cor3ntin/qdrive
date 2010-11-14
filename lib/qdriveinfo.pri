QDRIVEINFO_LIBNAME = qdriveinfo

INCLUDEPATH *= $$PWD/../include

contains(CONFIG, staticlib) {
    DEFINES -= QDRIVEINFO_DLL
} else {
    DEFINES *= QDRIVEINFO_DLL
}

CONFIG(debug, debug|release) {
    unix {
        QDRIVEINFO_LIBNAME = $$join(QDRIVEINFO_LIBNAME,,,_debug)
    } else {
        QDRIVEINFO_LIBNAME = $$join(QDRIVEINFO_LIBNAME,,,d)
    }
}

QDRIVEINFO_LIBS = -L$$PWD -L$$OUT_PWD -l$$QDRIVEINFO_LIBNAME

QMAKE_LIBDIR *= $$DESTDIR $$PWD $$OUT_PWD
