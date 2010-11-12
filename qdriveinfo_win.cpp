#include "qdriveinfo_p.h"

#include <QtCore/QDir>

static inline bool isRelativePath(const QString &path)
{
    // drive, e.g. "a:", or UNC root, e.q. "//"
    return !(path.startsWith(QLatin1Char('/'))
             || (path.length() >= 2
                 && ((path.at(0).isLetter() && path.at(1) == QLatin1Char(':'))
                     || (path.at(0) == QLatin1Char('/') && path.at(1) == QLatin1Char('/')))));
}

void QDriveInfoPrivate::initRootPath()
{
    if (data->rootPath.isEmpty())
        return;

    if (isRelativePath(data->rootPath)) {
        data->rootPath.clear();
        return;
    }

    // ### test when disk mounted in folder on other disk
    QString path = QDir::toNativeSeparators(data->rootPath);
    wchar_t buffer[MAX_PATH + 1];
    if (::GetVolumePathName((wchar_t *)path.utf16(), buffer, MAX_PATH)) {
        data->rootPath = QDir::fromNativeSeparators(QString::fromWCharArray(buffer));
        if (!data->rootPath.endsWith(QLatin1Char('/')))
            data->rootPath.append(QLatin1Char('/'));
    } else {
        data->rootPath.clear();
    }
}

static inline QDriveInfo::DriveType determineType(const QString &rootPath)
{
#if !defined(Q_OS_WINCE)
    UINT oldmode = ::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

    UINT result = ::GetDriveType((wchar_t *)rootPath.utf16());
    switch (result) {
    case DRIVE_REMOVABLE:
        return QDriveInfo::RemovableDrive;

    case DRIVE_FIXED:
        return QDriveInfo::InternalDrive;

    case DRIVE_REMOTE:
        return QDriveInfo::RemoteDrive;

    case DRIVE_CDROM:
        return QDriveInfo::CdromDrive;

    case DRIVE_RAMDISK:
        return QDriveInfo::RamDrive;

    case DRIVE_UNKNOWN:
    case DRIVE_NO_ROOT_DIR:
    // fall through
    default:
        break;
    };

    ::SetErrorMode(oldmode);
#else
    Q_UNUSED(rootPath)
#endif
    return QDriveInfo::InvalidDrive;
}

void QDriveInfoPrivate::doStat(uint requiredFlags)
{
    if (data->getCachedFlag(requiredFlags))
        return;

    if (!data->getCachedFlag(CachedRootPathFlag)) {
        initRootPath();
        data->setCachedFlag(CachedRootPathFlag);
    }

    if (data->rootPath.isEmpty() || (data->getCachedFlag(CachedValidFlag) && !data->valid))
        return;

    if (!data->getCachedFlag(CachedValidFlag))
        requiredFlags |= CachedValidFlag; // force drive validation


    uint bitmask = 0;

    bitmask = CachedValidFlag | CachedReadyFlag |
              CachedNameFlag | CachedFileSystemNameFlag;
    if (requiredFlags & bitmask) {
        getVolumeInformation();
        if (data->valid && !data->ready)
            bitmask = CachedValidFlag;
        data->setCachedFlag(bitmask);

        if (!data->valid)
            return;
    }

    bitmask = CachedAvailableSizeFlag | CachedFreeSizeFlag | CachedSizeFlag;
    if (requiredFlags & bitmask) {
        getDiskFreeSpace();
        data->setCachedFlag(bitmask);
    }

    bitmask = CachedDeviceFlag;
    if (requiredFlags & bitmask) {
        getDevice();
        data->setCachedFlag(bitmask);
    }

    bitmask = CachedTypeFlag;
    if (requiredFlags & bitmask) {
        data->type = determineType(data->rootPath);
        data->setCachedFlag(bitmask);
    }
}

void QDriveInfoPrivate::getVolumeInformation()
{
    UINT oldmode = ::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

    QString path = QDir::toNativeSeparators(data->rootPath);
    wchar_t nameBuf[MAX_PATH + 1];
    wchar_t fileSystemNameBuf[MAX_PATH + 1];
    bool result = ::GetVolumeInformation((wchar_t *)path.utf16(),
                                         nameBuf, MAX_PATH,
                                         0, 0, 0,
                                         fileSystemNameBuf, MAX_PATH);
    if (!result) {
        data->ready = false;
        data->valid = (::GetLastError() == ERROR_NOT_READY);
    } else {
        data->ready = true;
        data->valid = true;

        data->name = QString::fromWCharArray(nameBuf);
        data->fileSystemName = QString::fromWCharArray(fileSystemNameBuf);
    }

    ::SetErrorMode(oldmode);
}

void QDriveInfoPrivate::getDiskFreeSpace()
{
    UINT oldmode = ::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

    QString path = QDir::toNativeSeparators(data->rootPath);
    ::GetDiskFreeSpaceEx((wchar_t *)path.utf16(),
                         (PULARGE_INTEGER)&data->availableSize,
                         (PULARGE_INTEGER)&data->totalSize,
                         (PULARGE_INTEGER)&data->freeSize);

    ::SetErrorMode(oldmode);
}

void QDriveInfoPrivate::getDevice()
{
    QString path = QDir::toNativeSeparators(data->rootPath);
    wchar_t deviceBuffer[MAX_PATH + 1];
    if (::GetVolumeNameForVolumeMountPoint((wchar_t *)path.utf16(), deviceBuffer, MAX_PATH))
        data->device = QString::fromWCharArray(deviceBuffer);
}

QList<QDriveInfo> QDriveInfoPrivate::drives()
{
    QList<QDriveInfo> drives;

    char driveName[] = "A:/";
    quint32 driveBits = quint32(::GetLogicalDrives()) & 0x3ffffff;
    while (driveBits) {
        if (driveBits & 1)
            drives.append(QDriveInfo(QLatin1String(driveName)));
        driveName[0]++;
        driveBits = driveBits >> 1;
    }

    return drives;
}
