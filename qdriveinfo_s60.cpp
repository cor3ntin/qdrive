#include "qdriveinfo_p.h"

#include <f32file.h>

// ### remove following line and include private header
Q_CORE_EXPORT RFs &qt_s60GetRFs();

void QDriveInfoPrivate::initRootPath()
{
    if (data->rootPath.isEmpty())
        return;

    QChar driveLetter = data->rootPath.at(0).toUpper();

    RFs rfs = qt_s60GetRFs();

    TInt drive;
    if (RFs::CharToDrive(driveLetter.toAscii(), drive) == KErrNone) {
        data->rootPath = driveLetter + QLatin1String(":/");
        data->device = QByteArray(1, drive);
    } else {
        data->rootPath.clear();
    }
}

void QDriveInfoPrivate::doStat(uint requiredFlags)
{
    if (data->getCachedFlag(requiredFlags))
        return;

    if (!data->getCachedFlag(CachedRootPathFlag)) {
        initRootPath();
        data->setCachedFlag(CachedRootPathFlag | CachedDeviceFlag);
    }

    if (data->rootPath.isEmpty() || (data->getCachedFlag(CachedValidFlag) && !data->valid))
        return;

    if (!data->getCachedFlag(CachedValidFlag))
        requiredFlags |= CachedValidFlag; // force drive validation


    uint bitmask = CachedFileSystemNameFlag | CachedNameFlag |
                   CachedTotalSizeFlag | CachedFreeSizeFlag | CachedAvailableSizeFlag |
                   CachedTypeFlag | CachedCapabilitiesFlag |
                   CachedReadyFlag | CachedValidFlag;
    if (requiredFlags & bitmask) {
        getVolumeInfo();
        data->setCachedFlag(bitmask);
    }
}

void QDriveInfoPrivate::getVolumeInfo()
{
    RFs rfs = qt_s60GetRFs();

    TInt drive = data->device[0];

    TVolumeInfo volumeInfo;
    if (rfs.Volume(volumeInfo, drive) != KErrNone) {
        data->valid = true;
        data->ready = true;

        data->name = QString::fromUtf16((const ushort *)volumeInfo.iName.Ptr(), volumeInfo.iName.Length());
        TFSName fileSystemName;
        if (rfs.FileSystemSubType(drive, fileSystemName) == KErrNone)
            data->fileSystemName = QString::fromUtf16((const ushort *)fileSystemName.Ptr(), fileSystemName.Length());

        data->totalSize = volumeInfo.iSize;
        data->freeSize = volumeInfo.iFree;
        data->availableSize = volumeInfo.iFree;

        switch (volumeInfo.iDrive.iType) {
        case EMediaFlash:
        case EMediaFloppy:
            data->type = QDriveInfo::RemovableDrive;
            break;
        case EMediaHardDisk:
            data->type = QDriveInfo::InternalDrive;
            break;
        case EMediaNANDFlash:
            data->type = QDriveInfo::InternalFlashDrive;
            break;
        case EMediaRam:
        case EMediaRom:
            data->type = QDriveInfo::RamDrive;
            break;
        case EMediaCdRom:
            data->type = QDriveInfo::CdromDrive;
            break;
        case EMediaRemote:
            data->type = QDriveInfo::RemoteDrive;
            break;
        case EMediaNotPresent:
        default:
            break;
        }

        if (volumeInfo.iDrive.iMediaAtt & KMediaAttWriteProtected)
            data->capabilities |= QDriveInfo::ReadOnlyVolume;
    }
}

QList<QDriveInfo> QDriveInfoPrivate::drives()
{
    QList<QDriveInfo> drives;

    RFs rfs = qt_s60GetRFs();

    TDriveList drivelist;
    if (rfs.DriveList(drivelist) == KErrNone) {
        for (int i = EDriveA; i < EDriveZ; ++i) {
            if (drivelist[i]) {
                TChar driveChar;
                if (RFs::DriveToChar(i, driveChar) == KErrNone)
                    drives.append(QDriveInfo(QChar(driveChar).toUpper() + QLatin1String(":/")));
            }
        }
    }

    return drives;
}
