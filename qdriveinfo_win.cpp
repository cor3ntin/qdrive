#include "qdriveinfo.h"
#include "qdriveinfo_p.h"

QList<QDriveInfo> QDriveInfoPrivate::drives()
{
    QList<QDriveInfo> drives;

    char driveName[] = "A:/";
    quint32 driveBits = (quint32)::GetLogicalDrives() & 0x3ffffff;
    while (driveBits) {
        if (driveBits & 1)
            drives.append(QDriveInfo(QLatin1String(driveName)));
        driveName[0]++;
        driveBits = driveBits >> 1;
    }

    return drives;
}

void QDriveInfoPrivate::doStat(uint requiredFlags)
{
    requiredFlags |= CachedValidFlag; // force drive validation

    if (data->getCachedFlag(requiredFlags))
        return;

    uint bitmask = 0;
    bitmask = CachedValidFlag | CachedReadyFlag |
              CachedNameFlag | CachedFileSystemNameFlag;
    if (requiredFlags & bitmask) {
        getVolumeInformation();
        if (data->valid && !data->ready)
            bitmask = CachedValidFlag;
        data->setCachedFlag(bitmask);
    }

    if (!data->valid)
        return;

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
        getType();
        data->setCachedFlag(bitmask);
    }

    bitmask = CachedRootPathFlag;
    if (requiredFlags & bitmask) {
        getPath();
        data->setCachedFlag(bitmask);
    }
}

void QDriveInfoPrivate::getVolumeInformation()
{
    UINT oldmode = ::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

    wchar_t nameBuf[MAX_PATH + 1];
    wchar_t fileSystemNameBuf[MAX_PATH + 1];
    bool result = ::GetVolumeInformation((wchar_t *)data->rootPath.utf16(),
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

    ::GetDiskFreeSpaceEx((wchar_t *)data->rootPath.utf16(),
                         (PULARGE_INTEGER)&data->availableSize,
                         (PULARGE_INTEGER)&data->totalSize,
                         (PULARGE_INTEGER)&data->freeSize);

    ::SetErrorMode(oldmode);
}

void QDriveInfoPrivate::getDevice()
{
    UINT oldmode = ::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

    wchar_t deviceBuffer[MAX_PATH + 1];
    bool result = ::GetVolumeNameForVolumeMountPoint((wchar_t *)data->rootPath.utf16(),
                                                     deviceBuffer, MAX_PATH);
    if (result)
        data->device = QString::fromWCharArray(deviceBuffer);

    ::SetErrorMode(oldmode);
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

void QDriveInfoPrivate::getType()
{
    doStat(CachedRootPathFlag); // we need a root path to get info

    data->type = determineType(data->rootPath);
}

void QDriveInfoPrivate::getPath()
{
    // TODO: test when disk mounted in folder on other disk
    wchar_t buffer[MAX_PATH + 1];
    bool result = ::GetVolumePathName((wchar_t *)data->rootPath.utf16(), buffer, MAX_PATH);
    if (result)
        data->rootPath = QString::fromWCharArray(buffer);
}
