/****************************************************************************
**
** Copyright (C) 2012 Ivan Komissarov
** Contact: http://www.qt-project.org/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qdriveinfo.h"
#include "qdriveinfo_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QDriveInfo
    \reentrant
    \brief QDriveInfo provides information about currently mounted drives and volumes.

    \ingroup io
    \ingroup shared

    It allows to retrieve information about drive's space, its mount point, label, filesystem
    name and type.

    You can create QDriveInfo and pass the path to the drive's mount point as the constructor
    parameter, or you can set it using setRootPath() method. Also, you can get the list of all
    mounted filesystems using drives() method.

    QDriveInfo always caches the retrieved information but you can call refresh() to
    invalidate the cache.

    The following example retrieves the most common information about a root drive of
    the system and prints information about it.

    \snippet doc/src/snippets/code/src_corelib_io_qdriveinfo.cpp 2

*/

/*!
    \enum QDriveInfo::DriveType
    This enum describes the type of a drive or volume

    \value UnknownDrive          Drive type cannot be determined.
    \value InternalDrive         Is internal mass storage drive like a hard drive.
    \value RemovableDrive        Is a removable disk like flash disk or MMC.
    \value RemoteDrive           Is a network drive.
    \value CdromDrive            Is a CD ROM or DVD drive.
    \value InternalFlashDrive    Is an internal flash disk, or Phone Memory.
    \value RamDrive              Is a virtual drive made in RAM memory.
*/

/*!
    Constructs an empty QDriveInfo object.

    This object is not ready for use, invalid and all its parameters are empty.

    \sa setRootPath(), isReady(), isValid()
*/
QDriveInfo::QDriveInfo()
    : d_ptr(new QDriveInfoPrivate)
{
}

/*!
    Constructs a new QDriveInfo that gives information about the drive, mounted at
    \a rootPath.

    If you pass a directory or file, the QDriveInfo object will refer to the volume where
    this directory or file is located.
    You can check if \a rootPath is correct using isValid() method.

    The following example shows how to get drive on which application is located.
    It is a goods idea always check that drive is ready and valid.

    \snippet doc/src/snippets/code/src_corelib_io_qdriveinfo.cpp 0

    \sa setRootPath()
*/
QDriveInfo::QDriveInfo(const QString &rootPath)
    : d_ptr(new QDriveInfoPrivate)
{
    d_ptr->rootPath = rootPath;
}

/*!
    Constructs a new QDriveInfo that is a copy of the \a other QDriveInfo.
*/
QDriveInfo::QDriveInfo(const QDriveInfo &other)
    : d_ptr(other.d_ptr)
{
}

/*!
    Destroys the QDriveInfo and frees its resources.
*/
QDriveInfo::~QDriveInfo()
{
}

/*!
    Makes a copy of the \a other QDriveInfo and assigns it to this QDriveInfo.
*/
QDriveInfo &QDriveInfo::operator=(const QDriveInfo &other)
{
    if (this != &other)
        d_ptr = other.d_ptr;
    return *this;
}

/*!
    \fn bool QDriveInfo::operator!=(const QDriveInfo &other) const

    Returns true if this QDriveInfo object refers to a different drive or volume
    than the one specified by \a other; otherwise returns false.

    \sa operator==()
*/

/*!
    Returns true if this QDriveInfo object refers to the same drive or volume
    as the \a other; otherwise returns false.

    Note that the result of comparing two invalid QDriveInfo objects is always positive.

    \sa operator!=()
*/
bool QDriveInfo::operator==(const QDriveInfo &other) const
{
    if (d_ptr == other.d_ptr)
        return true;
    return device() == other.device();
}

/*!
    Returns mount point of the filesystem this QDriveInfo object represents.

    On Windows, returns drive letter in case the drive is not mounted to directory.

    Note that the value returned by rootPath() is the real mount point of a drive
    and may not be equal to the value passed to constructor or setRootPath() method.
    For example, if you have only the root drive in the system and pass '/directory'
    to setRootPath(), then this method will return '/'.

    \sa setRootPath(), device()
*/
QString QDriveInfo::rootPath() const
{
    Q_D(const QDriveInfo);
    const_cast<QDriveInfoPrivate *>(d)->doStat(QDriveInfoPrivate::CachedRootPathFlag);
    return d->rootPath;
}

/*!
    Sets QDriveInfo to the filesystem mounted at \a rootPath.

    You can also pass a path to the directory on the drive, in that case the rootPath()
    will return path that represents the drive's mount point.

    \sa rootPath()
*/
void QDriveInfo::setRootPath(const QString &rootPath)
{
    if (d_ptr->rootPath == rootPath)
        return;

    d_ptr.detach();
    Q_D(QDriveInfo);
    d->clear();
    d->rootPath = rootPath;
}

/*!
    Returns the size (in bytes) available for current user. If user is a root, returns all available size.

    This size can be less thant or equal to the free size, returned by bytesFree() function.

    \sa bytesTotal(), bytesFree()
*/
quint64 QDriveInfo::bytesAvailable() const
{
    Q_D(const QDriveInfo);
    const_cast<QDriveInfoPrivate *>(d)->doStat(QDriveInfoPrivate::CachedBytesAvailableFlag);
    return d->bytesAvailable;
}

/*!
    Returns the free size (in bytes) available on drive. Note, that if there is some kind
    of quotas on the filesystem, this value can be bigger than bytesAvailable().

    \sa bytesTotal(), bytesAvailable()
*/
quint64 QDriveInfo::bytesFree() const
{
    Q_D(const QDriveInfo);
    const_cast<QDriveInfoPrivate *>(d)->doStat(QDriveInfoPrivate::CachedBytesFreeFlag);
    return d->bytesFree;
}

/*!
    Returns total drive size in bytes.

    \sa bytesFree(), bytesAvailable()
*/
quint64 QDriveInfo::bytesTotal() const
{
    Q_D(const QDriveInfo);
    const_cast<QDriveInfoPrivate *>(d)->doStat(QDriveInfoPrivate::CachedBytesTotalFlag);
    return d->bytesTotal;
}

/*!
    Returns the name of filesystem.

    This is a platform-dependent function, and filesystem names can vary between different
    operation systems. For example, on Windows filesystem can be named as 'NTFS' and on Linux
    as 'ntfs-3g' or 'fuseblk'.

    \sa name()
*/
QByteArray QDriveInfo::fileSystemName() const
{
    Q_D(const QDriveInfo);
    const_cast<QDriveInfoPrivate *>(d)->doStat(QDriveInfoPrivate::CachedFileSystemNameFlag);
    return d->fileSystemName;
}

/*!
    Returns the device for this drive.

    The result of this function is platform-dependent and usually should not be used. However,
    you can retrieve this value for some platform-specific notes. By example, you can get device
    on Unix and try to read from it manually.

    On Unix filesystems (including Mac OS), this returns devpath like '/dev/sda0' for local drives.

    On Windows, returns UNC path starting with \\\\?\\ for local drives (i.e. volume GUID).

    \sa rootPath()
*/
QByteArray QDriveInfo::device() const
{
    Q_D(const QDriveInfo);
    const_cast<QDriveInfoPrivate *>(d)->doStat(QDriveInfoPrivate::CachedDeviceFlag);
    return d->device;
}

/*!
    Returns human-readable name of a filesystem, usually called 'label'.

    Not all filesystems support this feature, in this case value returned by this method could
    be empty. Also, empty string is returned if no label is set for drive.

    On Linux, retrieving the drive's label requires udev to be present in the system.

    \sa fileSystemName()
*/
QString QDriveInfo::name() const
{
    Q_D(const QDriveInfo);
    const_cast<QDriveInfoPrivate *>(d)->doStat(QDriveInfoPrivate::CachedNameFlag);
    return d->name;
}

/*!
    \fn bool QDriveInfo::isRoot() const

    Returns true if this QDriveInfo represents the system root volume or drive; false otherwise.

    On Unix filesystems, root drive is a drive mounted at "/", on Windows root drive is a drive
    where OS is installed.

    \sa rootDrive()
*/

/*!
    Returns true if the current filesystem is protected from writing; false otherwise.
*/
bool QDriveInfo::isReadOnly() const
{
    Q_D(const QDriveInfo);
    const_cast<QDriveInfoPrivate *>(d)->doStat(QDriveInfoPrivate::CachedReadOnlyFlag);
    return d->readOnly;
}

/*!
    Returns true if current filesystem is ready to work; false otherwise.

    Note that fileSystemName(), name(), bytesTotal(), bytesFree(), and bytesAvailable()
    will return an invalid data until drive is ready.

    \sa isValid()
*/
bool QDriveInfo::isReady() const
{
    Q_D(const QDriveInfo);
    const_cast<QDriveInfoPrivate *>(d)->doStat(QDriveInfoPrivate::CachedReadyFlag);
    return d->ready;
}

/*!
    Returns true if QDriveInfo specified by rootPath exists and is mounted correctly.

    \sa isReady()
*/
bool QDriveInfo::isValid() const
{
    Q_D(const QDriveInfo);
    const_cast<QDriveInfoPrivate *>(d)->doStat(QDriveInfoPrivate::CachedValidFlag);
    return d->valid;
}

/*!
    Returns the type of filesystem (i.e. remote drive, removable and so on).

    The following example prints the type of a drive.

    \snippet doc/src/snippets/code/src_corelib_io_qdriveinfo.cpp 3

    \sa QDriveInfo::DriveType
*/
QDriveInfo::DriveType QDriveInfo::type() const
{
    Q_D(const QDriveInfo);
    const_cast<QDriveInfoPrivate *>(d)->doStat(QDriveInfoPrivate::CachedTypeFlag);
    return QDriveInfo::DriveType(d->type);
}

/*!
    Returns the flags supported by drive's filesystem.
*/
QDriveInfo::Capabilities QDriveInfo::capabilities() const
{
    Q_D(const QDriveInfo);
    const_cast<QDriveInfoPrivate *>(d)->doStat(QDriveInfoPrivate::CachedCapabilitiesFlag);
    return d->capabilities;
}

/*!
    \fn bool QDriveInfo::hasCapability(QDriveInfo::Capability capability) const

    Returns true if drive's filesystem supports specified \a capability.
*/

/*!
    Resets QDriveInfo's inner cache.

    QDriveInfo caches information about drives to speed up performance. Some information can be
    retrieved by only one native function call (for example, if you call bytesTotal(),
    QDriveInfo will also cache information for bytesAvailable() and bytesFree()).
    Also, QDriveInfo won't update information for future calls and you have to manually
    reset cache when needed.
*/
void QDriveInfo::refresh()
{
    // do not detach
    d_func()->clear();
}

/*!
    Returns list of QDriveInfo's that corresponds to the list of currently mounted filesystems.

    On Windows, this returns drives presented in 'My Computer' folder. On Unix operation systems,
    returns list of all mounted filesystems (except for Mac, where devfs is ignored). In Linux, you
    will get a lot of pseudo filesystems by calling this function, but you can filter them out
    by using type() (as they always have an UnknownDrive type).

    The example shows how to retrieve all drives present in system and skip read-only drives.

    \snippet doc/src/snippets/code/src_corelib_io_qdriveinfo.cpp 1

    \sa rootDrive()
*/
QList<QDriveInfo> QDriveInfo::drives()
{
    return QDriveInfoPrivate::drives();
}

Q_GLOBAL_STATIC_WITH_ARGS(QDriveInfo, rootDrive, (QDriveInfoPrivate::rootDrive()))

/*!
    Returns a QDriveInfo object that represents the system root volume or drive.

    On Unix systems this call returns '/' volume, on Windows - volume where operating
    system is installed.

    \sa isRoot()
*/
QDriveInfo QDriveInfo::rootDrive()
{
    return *::rootDrive();
}

QT_END_NAMESPACE
