#ifndef QDRIVE_GLOBAL_H
#define QDRIVE_GLOBAL_H

#ifdef QDRIVEINFO_DLL
#  include <QtCore/QtGlobal>
#  ifdef QDRIVEINFO_MAKEDLL
#    define QDRIVEINFO_EXPORT Q_DECL_EXPORT
#  else
#    define QDRIVEINFO_EXPORT Q_DECL_IMPORT
#  endif
#endif
#ifndef QDRIVEINFO_EXPORT
#  define QDRIVEINFO_EXPORT
#endif

#endif // QDRIVE_GLOBAL_H
