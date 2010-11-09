#include "qdriveinfo.h"
#include "qdriveinfo_p.h"

#include <QDebug>

//================================== QDriveInfo ==================================

QDriveInfo::QDriveInfo():
        d_ptr(new QDriveInfoPrivate())
{
//    qDebug() << "QDriveInfo::QDriveInfo" << d_ptr->data->ref;
}

QDriveInfo::QDriveInfo(const QString &rootPath):
        d_ptr(new QDriveInfoPrivate())
{
//    Q_D(QDriveInfo);

    setRootPath(rootPath);
//    qDebug() << "QDriveInfo::QDriveInfo" << d_ptr->data->ref;
//    d->rootPath = QDir::toNativeSeparators(rootPath);
}

QDriveInfo::QDriveInfo(const QDriveInfo &other) :
        d_ptr(new QDriveInfoPrivate(other.d_ptr))
{
//    qDebug() << "QDriveInfo::QDriveInfo:copy" << d_ptr->data->ref;
//    d_ptr->data = other.d_ptr->data;
//    qDebug() << this->d_ptr->data->ref;
}

QDriveInfo &QDriveInfo::operator=(const QDriveInfo &other)
{
    if (this != &other)
        d_ptr->data.operator=(other.d_ptr->data);
    qDebug() << "QDriveInfo::operator=" << d_ptr->data->ref;
    return *this;
}

QDriveInfo::~QDriveInfo()
{
    delete d_ptr;
}

QList<QDriveInfo> QDriveInfo::drives()
{
    return QDriveInfoPrivate::drives();
}

quint64 QDriveInfo::availableSize() const
{
    const_cast<QDriveInfoPrivate*>(d_func())->stat(QDriveInfoPrivate::CachedAvailableSizeFlag);
    return d_func()->data->availableSize;
}

quint64 QDriveInfo::freeSize() const
{
    const_cast<QDriveInfoPrivate*>(d_func())->stat(QDriveInfoPrivate::CachedFreeSizeFlag);
    return d_func()->data->freeSize;
}

quint64 QDriveInfo::totalSize() const
{
    const_cast<QDriveInfoPrivate*>(d_func())->stat(QDriveInfoPrivate::CachedSizeFlag);
    return d_func()->data->totalSize;
}

QString QDriveInfo::fileSystemName() const
{
    const_cast<QDriveInfoPrivate*>(d_func())->stat(QDriveInfoPrivate::CachedFileSystemNameFlag);
    return d_func()->data->fileSystemName;
}

QString QDriveInfo::device() const
{
    const_cast<QDriveInfoPrivate*>(d_func())->stat(QDriveInfoPrivate::CachedDeviceFlag);
    return d_func()->data->device;
}

QString QDriveInfo::name() const
{
    const_cast<QDriveInfoPrivate*>(d_func())->stat(QDriveInfoPrivate::CachedNameFlag);
    return d_func()->data->name;
}

bool QDriveInfo::ready() const
{
    const_cast<QDriveInfoPrivate*>(d_func())->stat(QDriveInfoPrivate::CachedReadyFlag);
    return d_func()->data->ready;
}

bool QDriveInfo::isValid() const
{
    const_cast<QDriveInfoPrivate*>(d_func())->stat(QDriveInfoPrivate::CachedValidFlag);
    return d_func()->data->valid;
}

QString QDriveInfo::rootPath() const
{
    const_cast<QDriveInfoPrivate*>(d_func())->stat(QDriveInfoPrivate::CachedRootPathFlag);
    return d_func()->data->rootPath;
}

void QDriveInfo::setRootPath(const QString &rootPath)
{
    d_func()->stat(QDriveInfoPrivate::CachedRootPathFlag);
    d_func()->setRootPath(rootPath);
}

QDriveInfo::DriveType QDriveInfo::type() const
{
    const_cast<QDriveInfoPrivate*>(d_func())->stat(QDriveInfoPrivate::CachedTypeFlag);
    return d_func()->data->type;
}

//================================== QDriveInfoPrivate ==================================

QDriveInfoPrivate::QDriveInfoPrivate():
        data(new Data)
{
    data->availableSize = 0;
    data->freeSize = 0;
    data->totalSize = 0;
    data->ready = false;

    data->cache_enabled = true;
    data->cachedFlags = 0;
}

QDriveInfoPrivate::QDriveInfoPrivate(QDriveInfoPrivate *other):
        data(other->data)
{
}

void QDriveInfoPrivate::setRootPath(const QString &rootPath)
{
    data.detach();
    data->rootPath = rootPath;
    qDebug() << "QDriveInfo::setRootPath" << data->ref;
}
