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
    \brief QDriveInfo provides provides information about currently mounted drives or volumes.

    \ingroup io
    \ingroup shared

    QDriveInfo provides provides information about currently mounted drives or volumes.
    It gives you information about drive's space, it's mount point, label, filesystem name and type.

    You can create QDriveInfo and pass path to drive's mount point as a constructor parameter,
    or you can set it via setRootPath() method. Also, you can get all filesystems mounted in
    system using drives() method.

    QDriveInfo always caches the retreived information but you can call refresh() to invalidate the cache.
*/

/*!
    \enum QDriveInfo::DriveType
    This enum describes the type of drive or volume

    \value InvalidDrive          Drive type undetermined.
    \value InternalDrive         Is internal mass storage drive like a harddrive.
    \value RemovableDrive        Is a removable disk like flash or MMC.
    \value RemoteDrive           Is a network drive.
    \value CdromDrive            Is a cd rom drive.
    \value InternalFlashDrive    Is an internal flash disk, or Phone Memory.
    \value RamDrive              Is a virtual drive made in RAM memory.
*/

/*!
    \enum QDriveInfo::Capability
    Specifies the system capabilities of the drive or volume this QDriveInfo instance represents.
    The possible values are:

    \value CaseSensitiveFileNames     The specified volume supports case sensitivity for file names.
                                      Some environments cannot create files whose names differ only by case even if
                                      the file system was designed to support case sensitivity for file names.
                                      For example, the NTFS file system itself supports case sensitivity for file names
                                      but the Win32 environment subsystem does not for compatibility reasons,
                                      but being mounted on Unix NTFS can have case-sensitivity.
    \value AccessControlListsSupport  The specified volume preserves and enforces access control lists (ACL).
                                      For example, the NTFS, XFS and BTRFS file systems preserves and enforces ACLs,
                                      and the FATand UDF file system does not.
    \value ReadOnlyVolume             The specified volume is read-only.
    \value HardlinksSupport           The specified volume supports hard links.
    \value SymlinksSupport            The specified volume supports symbolic links (aka soft links).
*/

/*!
    Constructs an empty QDriveInfo object.

    This object is not ready, invalid and all it's parameters are empty.

    \sa setRootPath(), ready(), valid()
*/
QDriveInfo::QDriveInfo()
    : d_ptr(new QDriveInfoPrivate)
{
}

/*!
    Constructs a new QDriveInfo that gives information about the drive, mounted at
    \a rootPath.

    You can check if \a rootPath is correct using valid() method.

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

    Note, that value, returned by this function, is the real mount point of a drive and may it not be
    equal to value passed to constructor or to setRootPath() function. By example, if you have only
    root drive in system and you pass '/folder' to constructor, this funtion will return '/'.

    On Symbian, root path, passed to constructor or setRootPath() function, is always truncated to
    'X:/' where X is drive letter.
*/
QString QDriveInfo::rootPath() const
{
    const_cast<QDriveInfoPrivate*>(d_func())->doStat(QDriveInfoPrivate::CachedRootPathFlag);
    return d_func()->data->rootPath;
}

/*!
    Sets QDriveInfo to filesystem, mounted at \a rootPath.

    You can also pass a folder on the drive, in that case root path will be truncated to real mount
    point of the drive (if exists).

    \sa rootPath()
*/
void QDriveInfo::setRootPath(const QString &rootPath)
{
    Q_D(QDriveInfo);

    d->data.detach();
    d->data->clear();
    d->data->rootPath = rootPath;
}

/*!
    Returns size (in bytes) available for current user (not root).

    This size can be less than available size (exept for Symbian OS where these sizes are always equal).

    \sa bytesTotal(), bytesFree()
*/
quint64 QDriveInfo::bytesAvailable() const
{
    const_cast<QDriveInfoPrivate*>(d_func())->doStat(QDriveInfoPrivate::CachedBytesAvailableFlag);
    return d_func()->data->bytesAvailable;
}

/*!
    Returns free size (in bytes) available on drive. Note, that if there is some kind
    of qoutas on the filesystem, this value can be bigger than bytesAvailable()

    \sa bytesTotal(), bytesAvailable()
*/
quint64 QDriveInfo::bytesFree() const
{
    const_cast<QDriveInfoPrivate*>(d_func())->doStat(QDriveInfoPrivate::CachedBytesFreeFlag);
    return d_func()->data->bytesFree;
}

/*!
    Returns maximum drive size in bytes.

    \sa bytesFree(), bytesAvailable()
*/
quint64 QDriveInfo::bytesTotal() const
{
    const_cast<QDriveInfoPrivate*>(d_func())->doStat(QDriveInfoPrivate::CachedBytesTotalFlag);
    return d_func()->data->bytesTotal;
}

/*!
    Returns the name of filesystem.

    This is not a platform-independent function, and filesystem names can vary between different
    operation systems. For example, on Windows filesystem can be named as 'NTFS' and on Linux
    as 'ntfs-3g' or 'fuseblk'.
*/
QByteArray QDriveInfo::fileSystemName() const
{
    const_cast<QDriveInfoPrivate*>(d_func())->doStat(QDriveInfoPrivate::CachedFileSystemNameFlag);
    return d_func()->data->fileSystemName;
}

/*!
    Returns the device for this drive.

    The result of this function is platform-dependent and usually should not be used. However,
    you can retrieve this value for some platform-specific notes. By example, you can get device
    on Unix and try to read from it manually.

    On Unix filesystems (including Mac OS), this returns something like '/dev/sda0' for local drives.

    On Windows, returns UNC path, starting with \\?\ for local drives (i.e. volume GUID).

    On Symbian OS this function returns nothing.
*/
QByteArray QDriveInfo::device() const
{
    const_cast<QDriveInfoPrivate*>(d_func())->doStat(QDriveInfoPrivate::CachedDeviceFlag);
    return d_func()->data->device;
}

/*!
    Returns human-readable name of a filesystem, usually called as 'label'.

    Not all filesystems support this feature, so normally value, returned by this function could
    be empty. Also, empty string is returned if no label set for drive.

    Unfortunately, due to implementation, on Linux this function requires udev to be present in system.
    If there is no udev, returns empty string.

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
bool QDriveInfo::isReady() const
{
    const_cast<QDriveInfoPrivate*>(d_func())->doStat(QDriveInfoPrivate::CachedReadyFlag);
    return d_func()->data->ready;
}

/*!
    Returns true if QDriveInfo specified by rootPath exists and mounted correctly.

    \sa isReady()
*/
bool QDriveInfo::isValid() const
{
    const_cast<QDriveInfoPrivate*>(d_func())->doStat(QDriveInfoPrivate::CachedValidFlag);
    return d_func()->data->valid;
}

/*!
    Returns the type of filesystem (ie remote drive, removable and so on).

    \sa QDriveInfo::DriveType
*/
QDriveInfo::DriveType QDriveInfo::type() const
{
    const_cast<QDriveInfoPrivate*>(d_func())->doStat(QDriveInfoPrivate::CachedTypeFlag);
    return d_func()->data->type;
}

/*!
    Returns the capabilities supported by the current drive.
*/
QDriveInfo::Capabilities QDriveInfo::capabilities() const
{
    const_cast<QDriveInfoPrivate*>(d_func())->doStat(QDriveInfoPrivate::CachedCapabilitiesFlag);
    return QDriveInfo::Capabilities(d_func()->data->capabilities);
}

/*!
    Resets QDriveInfo inner cache.

    QDriveInfo caches information about drives to speed up performance. Some information can be retrieved
    by only 1 native funciton call, so, if you call bytesTotal(), QDriveInfo will also cache information
    for bytesAvailable() and bytesFree(). Also, QDriveInfo won't update information for future calls and
    you have to manually reset cache when needed.
*/
void QDriveInfo::refresh()
{
    d_func()->data->clear();
}

/*!
    Returns list of QDriveInfo's that corresponds to list of currently mounted filesystems.

    On Windows, this returnes drives presented in 'My Computer' folder. On Unix operation systems,
    returns list of all mounted filesystems (exept for Mac, where devfs is ignored). In Linux, you
    will get a lot of pseudo filesystems by calling this function, but you can filter them using
    type() (they always have InvalidDrive type) or by checking bytesTotal() (always equal to 0).
*/
QList<QDriveInfo> QDriveInfo::drives()
{
    return QDriveInfoPrivate::drives();
}
