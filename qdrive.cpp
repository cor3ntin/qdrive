#include "qdrive.h"
//#include "qdrive_p.h"

#include <QDebug>
#include <QDir>
//#include <stdio.h>

#ifdef Q_OS_LINUX
#if defined(Q_WS_MAEMO_5) || defined(Q_WS_MAEMO_6)
#include "qdrive_maemo_p.h"
#else
#include "qdrive_linux_p.h"
#endif //Q_WS_MAEMO_5 & Q_WS_MAEMO_6
#endif //Q_OS_LINUX

#ifdef Q_OS_WIN
#include "qdrive_win_p.h"
#endif
#ifdef Q_OS_MAC
#include "qdrive_mac_p.h"
#endif
#ifdef Q_OS_SYMBIAN
#include "qdrive_s60_p.h"
#endif

//================================== QDriveInfo ==================================

QDrive::QDrive(const QString &rootPath):
        d_ptr(new QDrivePrivate())
{
    Q_D(QDrive);
    d->cache_enabled = true;
    d->ready = false;
    d->cachedFlags = 0;
    d->availableSize = 0;
    d->freeSize = 0;
    d->size = 0;

    d->rootPath = QDir::toNativeSeparators(rootPath);
//    d->stat();

    startTimer(100);
}

QDrive::~QDrive()
{
    delete d_ptr;
}

quint64 QDrive::availableSize()
{
    d_func()->stat(QDrivePrivate::CachedAvailableSizeFlag);
    return d_func()->availableSize;
}

QString QDrive::fileSystemName()
{
    d_func()->stat(QDrivePrivate::CachedFileSystemNameFlag);
    return d_func()->fileSystemName;
}

quint64 QDrive::freeSize()
{
    d_func()->stat(QDrivePrivate::CachedFreeSizeFlag);
    return d_func()->freeSize;
}

QString QDrive::device() const
{
    const_cast<QDrivePrivate*>(d_func())->stat(QDrivePrivate::CachedDeviceFlag);
    return d_func()->device;
}

QString QDrive::name()
{
    d_func()->stat(QDrivePrivate::CachedNameFlag);
    return d_func()->name;
}

bool QDrive::setName(const QString &name)
{
    return d_func()->setName(name);
}

bool QDrive::ready()
{
    d_func()->stat(QDrivePrivate::CachedReadyFlag);
    return d_func()->ready;
}

QString QDrive::rootPath()
{
    d_func()->stat(QDrivePrivate::CachedRootPathFlag);
    return d_func()->rootPath;
}

quint64 QDrive::size()
{
    d_func()->stat(QDrivePrivate::CachedSizeFlag);
    return d_func()->size;
}

void QDrive::timerEvent(QTimerEvent *)
{
}

