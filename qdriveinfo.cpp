#include "qdriveinfo.h"
#include "qdriveinfo_p.h"

QDriveInfoPrivate::QDriveInfoPrivate()
    : data(new Data)
{
}

QDriveInfoPrivate::QDriveInfoPrivate(QDriveInfoPrivate *other)
    : data(other->data)
{
}


/*!
    \class QDriveInfo
    \reentrant
    \brief The QDriveInfo class provides information about filesystems mounted in system.

    \ingroup io
    \ingroup shared

    QDriveInfo provides information about currently present filesystems (or 'drives' on Windows).
    It gives you information about drive's space, it's mount point, filesystem type.

    You can create QDriveInfo and pass path to drives mount point as a constructor parameter,
    or you can set it via setRootPath() method. Also, you can get all filesystems mounted in
    system using drives() method().

    QDrive info caches information retreived about drives, but you can use refresh() to update
    infomation. Also, you can disable caching by calling setCaching(false).
*/

/*!
    Constructs an empty QDriveInfo object.

    \sa setRootPath()
*/
QDriveInfo::QDriveInfo()
    : d_ptr(new QDriveInfoPrivate)
{
}

/*!
    Constructs a new QDriveInfo that gives information about the drive, mounted at
    \a rootPath.

    \sa setRootPath()
*/
QDriveInfo::QDriveInfo(const QString &rootPath)
    : d_ptr(new QDriveInfoPrivate)
{
    setRootPath(rootPath);
}

/*!
    Constructs a new QDriveInfo that is a copy of the given \a other QDriveInfo.
*/
QDriveInfo::QDriveInfo(const QDriveInfo &other)
    : d_ptr(new QDriveInfoPrivate(other.d_ptr))
{
}

/*!
    Destroys the QDriveInfo and frees its resources.
*/
QDriveInfo::~QDriveInfo()
{
    delete d_ptr;
}

/*!
    Makes a copy of the given \a other QDriveInfo and assigns it to this QDriveInfo.
*/
QDriveInfo &QDriveInfo::operator=(const QDriveInfo &other)
{
    if (this != &other)
        d_ptr->data.operator=(other.d_ptr->data);
    return *this;
}

/*!
    Returns mount path of the filesystem, presented by QDriveInfo.
    On Windows, usually returns drive letter, in case drive is not mounted to specific folder.
*/
QString QDriveInfo::rootPath() const
{
    const_cast<QDriveInfoPrivate*>(d_func())->doStat(QDriveInfoPrivate::CachedRootPathFlag);
    return d_func()->data->rootPath;
}

/*!
    Sets QDriveInfo to filesystem, mounted at \a rootPath.
*/
void QDriveInfo::setRootPath(const QString &rootPath)
{
    Q_D(QDriveInfo);

    d->data.detach();
    d->data->rootPath = rootPath;
}

/*!
    Returns size (in bytes) available for current user.

    \sa freeSize(), totalSize()
*/
quint64 QDriveInfo::availableSize() const
{
    const_cast<QDriveInfoPrivate*>(d_func())->doStat(QDriveInfoPrivate::CachedAvailableSizeFlag);
    return d_func()->data->availableSize;
}

/*!
    Returns free size (in bytes) available on drive. Note, that if there is some kind
    of qoutas on the filesystem, this value can be bigger than availableSize()

    \sa availableSize(), totalSize()
*/
quint64 QDriveInfo::freeSize() const
{
    const_cast<QDriveInfoPrivate*>(d_func())->doStat(QDriveInfoPrivate::CachedFreeSizeFlag);
    return d_func()->data->freeSize;
}

/*!
    Returns maximum drive size in bytes.

    \sa availableSize(), freeSize()
*/
quint64 QDriveInfo::totalSize() const
{
    const_cast<QDriveInfoPrivate*>(d_func())->doStat(QDriveInfoPrivate::CachedSizeFlag);
    return d_func()->data->totalSize;
}

/*!
    Returns the name of filesystem.
    This is not a platform-ndependent function, and filesystem names can vary from different
    operation systems. For example, on Windows filesystem named as 'NTFS' and on Linux
    as 'ntfs-3g'
*/
QString QDriveInfo::fileSystemName() const
{
    const_cast<QDriveInfoPrivate*>(d_func())->doStat(QDriveInfoPrivate::CachedFileSystemNameFlag);
    return d_func()->data->fileSystemName;
}

/*!
    Returns the device for this drive.
    On Unix filesystems, this may return something like '/dev/sda0' for local drives.
    On Windows, returns UNC path, starting with \\?\ for local drives.
*/
QString QDriveInfo::device() const
{
    const_cast<QDriveInfoPrivate*>(d_func())->doStat(QDriveInfoPrivate::CachedDeviceFlag);
    return d_func()->data->device;
}

/*!
    Returns human-readable name of a filesystem, usually called as 'label'.
    Not all filesystems support this feature.
    Also, on Linux this function require udev be present in system. If there is no udev,
    returns empty string.

    Also, empty string is returned if no label set.
*/
QString QDriveInfo::name() const
{
    const_cast<QDriveInfoPrivate*>(d_func())->doStat(QDriveInfoPrivate::CachedNameFlag);
    return d_func()->data->name;
}

/*!
    Returns true is current filesystem is ready for work.
    This function can return false only on Windows for floppy or cdrom drives.
    Note, that you can't retreive any information about device if it is not ready.

    \sa isValid()
*/
bool QDriveInfo::ready() const
{
    const_cast<QDriveInfoPrivate*>(d_func())->doStat(QDriveInfoPrivate::CachedReadyFlag);
    return d_func()->data->ready;
}

/*!
    Returns true if QDriveInfo specified by rootPath exists and mounted correctly.

    \sa ready()
*/
bool QDriveInfo::isValid() const
{
    const_cast<QDriveInfoPrivate*>(d_func())->doStat(QDriveInfoPrivate::CachedValidFlag);
    return d_func()->data->valid;
}

/*!
    Returns type of filesystem (ie remote, removable and so on).

    \sa QDriveInfo::DriveType
*/
QDriveInfo::DriveType QDriveInfo::type() const
{
    const_cast<QDriveInfoPrivate*>(d_func())->doStat(QDriveInfoPrivate::CachedTypeFlag);
    return d_func()->data->type;
}

/*!
    Resets QDriveInfo inner cache.
*/
void QDriveInfo::refresh()
{
    d_func()->data->clear();
}

/*!
    Returns list of QDriveInfo's that corresponds to list of currently mounted filesystems.
*/
QList<QDriveInfo> QDriveInfo::drives()
{
    return QDriveInfoPrivate::drives();
}
