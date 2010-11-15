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

    TInt driveNumber;
    if (RFs::CharToDrive(driveLetter.toAscii(), driveNumber) == KErrNone) {
        data->rootPath = driveLetter + QLatin1String(":/");
        data->device = QByteArray(1, driveNumber);
    } else {
        data->rootPath.clear();
    }
}

void QDriveInfoPrivate::doStat(uint requiredFlags)
{
    if (data->getCachedFlag(requiredFlags))
        return;

    if (!data->getCachedFlag(CachedRootPathFlag | CachedDeviceFlag)) {
        initRootPath();
        data->setCachedFlag(CachedRootPathFlag | CachedDeviceFlag);
    }

    if (data->rootPath.isEmpty() || (data->getCachedFlag(CachedValidFlag) && !data->valid))
        return;

    if (!data->getCachedFlag(CachedValidFlag))
        requiredFlags |= CachedValidFlag; // force drive validation


    uint bitmask = CachedFileSystemNameFlag | CachedNameFlag |
                   CachedBytesTotalFlag | CachedBytesFreeFlag | CachedBytesAvailableFlag |
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
        TFSName fileSystemNameBuf;
        if (rfs.FileSystemSubType(drive, fileSystemNameBuf) == KErrNone)
            data->fileSystemName = QString::fromUtf16((const ushort *)fileSystemNameBuf.Ptr(), fileSystemNameBuf.Length()).toLatin1();

        data->bytesTotal = volumeInfo.iSize;
        data->bytesFree = volumeInfo.iFree;
        data->bytesAvailable = volumeInfo.iFree;

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

    char driveName[] = "A:/";
    TChar driveChar;
    TDriveList drivelist;
    if (rfs.DriveList(drivelist) == KErrNone) {
        for (TInt driveNumber = EDriveA; driveNumber <= EDriveZ; ++driveNumber) {
            if (drivelist[driveNumber] && RFs::DriveToChar(driveNumber, driveChar) == KErrNone) {
                driveName[0] = driveChar;
                QDriveInfo drive;
                drive.d_ptr->data->rootPath = QLatin1String(driveName);
                drive.d_ptr->data->device = QByteArray(1, driveNumber);
                drive.d_ptr->data->setCachedFlag(CachedRootPathFlag | CachedDeviceFlag);
                drives.append(drive);
            }
        }
    }

    return drives;
}

QDriveInfo QDriveInfoPrivate::rootDrive()
{
    RFs rfs = qt_s60GetRFs();

    char driveName[] = "A:/";
    driveName[0] = RFs::GetSystemDriveChar();
    TInt driveNumber;
    if (RFs::CharToDrive(driveName[0], driveNumber) == KErrNone) {
        QDriveInfo drive;
        drive.d_ptr->data->rootPath = QLatin1String(driveName);
        drive.d_ptr->data->device = QByteArray(1, driveNumber);
        drive.d_ptr->data->setCachedFlag(CachedRootPathFlag | CachedDeviceFlag);
        return drive;
    }

    return QDriveInfo();
}
