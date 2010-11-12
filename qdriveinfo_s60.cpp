#include "qdriveinfo.h"
#include "qdriveinfo_p.h"

#include <f32file.h>

// TODO: when moved to QtCore remove following line and include private header
Q_CORE_EXPORT RFs& qt_s60GetRFs();

QList<QDriveInfo> QDriveInfoPrivate::drives()
{
    QList<QDriveInfo> drives;

    RFs &rfs = qt_s60GetRFs();;
    TInt result;

    TDriveList drivelist;

    result = rfs.DriveList(drivelist);
    if (result != KErrNone) {
        return drives;
    }

    for (int i = 0; i < KMaxDrives; ++i) {
        if (drivelist[i] != 0) {
            TChar driveChar;
            result = RFs::DriveToChar(i, driveChar);
            if (result == KErrNone)
                drives.append(QDriveInfo(QChar(driveChar)));
        }
    }

    return drives;
}

void QDriveInfoPrivate::doStat(uint requiredFlags)
{
    if (data->getCachedFlag(requiredFlags))
        return;

    if (!data->getCachedFlag(CachedValidFlag))
        requiredFlags |= CachedValidFlag; // force drive validation

    uint bitmask = 0;

    bitmask = CachedAvailableSizeFlag | CachedFreeSizeFlag | CachedSizeFlag |
              CachedNameFlag | CachedValidFlag | CachedReadyFlag;
    if (requiredFlags & bitmask) {
        getVolumeInfo();
        data->setCachedFlag(bitmask);
    }

    bitmask = CachedFileSystemNameFlag;
    if (requiredFlags & bitmask) {
        getFileSystemName();
        data->setCachedFlag(bitmask);
    }

    bitmask = CachedTypeFlag;
    if (requiredFlags & bitmask) {
        getType();
        data->setCachedFlag(bitmask);
    }

    bitmask = CachedRootPathFlag;
    if (requiredFlags & bitmask) {
        getRootPath();
        data->setCachedFlag(bitmask);
    }
}

void QDriveInfoPrivate::getVolumeInfo()
{
    TInt drive;
    TInt result;
    TVolumeInfo volumeInfo;
    RFs &rfs = qt_s60GetRFs();

    if (data->rootPath.isEmpty())
        return;

    result = RFs::CharToDrive(TChar(data->rootPath[0].toAscii()), drive);
    if (result != KErrNone) {
        return;
    }

    result = rfs.Volume(volumeInfo, drive);
    if (result != KErrNone) {
        // do we really need this code?
        data->valid = false;
        data->ready = false;
        return;
    }
    data->valid = true;
    data->ready = true;

    data->availableSize = volumeInfo.iFree;
    data->freeSize = volumeInfo.iFree;
    data->totalSize = volumeInfo.iSize;
    // TODO: check this code
    data->name = QString::fromWCharArray((wchar_t *)volumeInfo.iName.Ptr(), KMaxFileName);
}

void QDriveInfoPrivate::getFileSystemName()
{
    TInt drive;
    TInt result;
    TFSName fileSystemName;
    RFs &rfs = qt_s60GetRFs();

    if (data->rootPath.isEmpty())
        return;

    result = RFs::CharToDrive(TChar(data->rootPath[0].toAscii()), drive);
    if (result != KErrNone) {
        return;
    }

    result = rfs.FileSystemName(fileSystemName, drive);
    if (result != KErrNone) {
        return;
    }

    // TODO: check this code
    data->fileSystemName = QString::fromWCharArray((wchar_t *)fileSystemName.Ptr(),
                                                   KMaxFSNameLength
                                                   );
}

void QDriveInfoPrivate::getRootPath()
{
    if (data->rootPath.length() == 1) {
        data->rootPath.append(":/");
    }
    else if (data->rootPath.length() == 2) {
        if (data->rootPath.at(1) == ':')
            data->rootPath.append('/');
    }
    else if (data->rootPath.length() >= 3) {
        if (data->rootPath.at(1) == ':' &&
            data->rootPath.at(2) == '/'
            )
            data->rootPath = data->rootPath.left(3); // Convert to X:/
    } else
        data->rootPath = ""; // invalid rootPath
}

// From Qt Mobility
inline static QDriveInfo::DriveType typeForDrive(const QString &driveVolume)
{
    if (driveVolume.size() != 1) {
        return QDriveInfo::InvalidDrive;
    }

    TInt drive;
    if (RFs::CharToDrive(TChar(driveVolume[0].toAscii()), drive) != KErrNone) {
        return QDriveInfo::InvalidDrive;
    }

    TDriveInfo driveInfo;
    RFs &rfs = qt_s60GetRFs();

    if (rfs.Drive(driveInfo, drive) != KErrNone) {
        return QDriveInfo::InvalidDrive;
    }

    if (driveInfo.iType == EMediaRemote) {
        return QDriveInfo::RemoteDrive;
    } else if (driveInfo.iType == EMediaCdRom) {
        return QDriveInfo::CdromDrive;
    }

    if (driveInfo.iDriveAtt & KDriveAttInternal) {
        return QDriveInfo::InternalDrive;
    } else if (driveInfo.iDriveAtt & KDriveAttRemovable) {
        return QDriveInfo::RemovableDrive;
    }

    return QDriveInfo::InvalidDrive;
}

void QDriveInfoPrivate::getType()
{
    data->type = typeForDrive(data->rootPath);
}
