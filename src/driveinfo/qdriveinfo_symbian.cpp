#include "qdriveinfo_p.h"

#include <f32file.h>

// ### remove following line and include private header
Q_CORE_EXPORT RFs &qt_s60GetRFs();

void QDriveInfoPrivate::initRootPath()
{
    if (rootPath.isEmpty())
        return;

    QChar driveLetter = rootPath.at(0).toUpper();
    rootPath.clear();

    RFs rfs = qt_s60GetRFs();

    TInt driveNumber;
    if (RFs::CharToDrive(driveLetter.toLatin1(), driveNumber) == KErrNone) {
        rootPath = driveLetter + QLatin1String(":/");
        device = QByteArray(1, driveNumber);
    }
}

void QDriveInfoPrivate::doStat(uint requiredFlags)
{
    if (getCachedFlag(requiredFlags))
        return;

    if (!getCachedFlag(CachedRootPathFlag | CachedDeviceFlag)) {
        initRootPath();
        setCachedFlag(CachedRootPathFlag | CachedDeviceFlag);
    }

    if (rootPath.isEmpty() || (getCachedFlag(CachedValidFlag) && !valid))
        return;

    if (!getCachedFlag(CachedValidFlag))
        requiredFlags |= CachedValidFlag; // force drive validation


    uint bitmask = CachedFileSystemNameFlag | CachedNameFlag |
                   CachedBytesTotalFlag | CachedBytesFreeFlag | CachedBytesAvailableFlag |
                   CachedTypeFlag | CachedReadOnlyFlag |
                   CachedReadyFlag | CachedValidFlag;
    if (requiredFlags & bitmask) {
        getVolumeInfo();
        if (valid && !ready)
            bitmask = CachedTypeFlag | CachedReadOnlyFlag | CachedValidFlag;
        setCachedFlag(bitmask);
    }
}

void QDriveInfoPrivate::getVolumeInfo()
{
    RFs rfs = qt_s60GetRFs();

    TInt drive = device[0];

    TDriveInfo driveInfo;
    if (rfs.Drive(driveInfo, drive) == KErrNone) {
        valid = true;

        TVolumeInfo volumeInfo;
        if (rfs.Volume(volumeInfo, drive) == KErrNone) {
            ready = true;

            name = QString::fromUtf16((const ushort *)volumeInfo.iName.Ptr(),
                                            volumeInfo.iName.Length());
            TFSName fileSystemNameBuf;
            if (rfs.FileSystemSubType(drive, fileSystemNameBuf) == KErrNone)
                fileSystemName = QString::fromUtf16((const ushort *)fileSystemNameBuf.Ptr(),
                                                          fileSystemNameBuf.Length()).toLatin1();

            bytesTotal = volumeInfo.iSize;
            bytesFree = volumeInfo.iFree;
            bytesAvailable = volumeInfo.iFree;
        }

        readOnly = (driveInfo.iMediaAtt & KMediaAttWriteProtected) != 0;

        switch (driveInfo.iType) {
        case EMediaFlash:
        case EMediaFloppy:
            type = QDriveInfo::RemovableDrive;
            break;
        case EMediaHardDisk:
        case EMediaRom:
            type = QDriveInfo::InternalDrive;
            if (driveInfo.iDriveAtt & KDriveAttRemovable)
                type = QDriveInfo::RemovableDrive;
            break;
        case EMediaNANDFlash:
            type = QDriveInfo::InternalFlashDrive;
            break;
        case EMediaRam:
            type = QDriveInfo::RamDrive;
            break;
        case EMediaCdRom:
            type = QDriveInfo::CdromDrive;
            readOnly = true;
            break;
        case EMediaRemote:
            type = QDriveInfo::RemoteDrive;
            break;
        case EMediaNotPresent:
        default:
            break;
        }
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
                drive.d_ptr->rootPath = QLatin1String(driveName);
                drive.d_ptr->device = QByteArray(1, driveNumber);
                drive.d_ptr->setCachedFlag(CachedRootPathFlag | CachedDeviceFlag);
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
        drive.d_ptr->rootPath = QLatin1String(driveName);
        drive.d_ptr->device = QByteArray(1, driveNumber);
        drive.d_ptr->setCachedFlag(CachedRootPathFlag | CachedDeviceFlag);
        return drive;
    }

    return QDriveInfo();
}
