#include "qdriveinfo.h"
#include "qdriveinfo_p.h"

#include <f32file.h>

// TODO: when moved to QtCore remove following line and include private header
Q_CORE_EXPORT RFs& qt_s60GetRFs();

QList<QDriveInfo> QDriveInfoPrivate::drives()
{
    QList<QDriveInfo> drives;

    RFs &rfs = qt_s60GetRFs();

    TDriveList drivelist;
    if (rfs.DriveList(drivelist) == KErrNone) {
        for (int i = 0; i < KMaxDrives; ++i) {
            if (drivelist[i] != 0) {
                TChar driveChar;
                result = RFs::DriveToChar(i, driveChar);
                if (result == KErrNone)
                    drives.append(QDriveInfo(QChar(driveChar).toUpper() + QLatin1String(":/")));
            }
        }
    }

    return drives;
}

void QDriveInfoPrivate::doStat(uint requiredFlags)
{
    if (data->getCachedFlag(requiredFlags))
        return;

    if (!data->getCachedFlag(CachedRootPathFlag)) {
        getRootPath();
        data->setCachedFlag(CachedRootPathFlag);
    }

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
}

void QDriveInfoPrivate::getVolumeInfo()
{
    RFs &rfs = qt_s60GetRFs();

    TInt drive;
    TVolumeInfo volumeInfo;
    if (RFs::CharToDrive(TChar(data->rootPath[0].toAscii()), drive) == KErrNone
        && rfs.Volume(volumeInfo, drive) != KErrNone) {
        // do we really need this code?
        data->valid = false;
        data->ready = false;
    } else {
        data->valid = true;
        data->ready = true;

        data->availableSize = volumeInfo.iFree;
        data->freeSize = volumeInfo.iFree;
        data->totalSize = volumeInfo.iSize;

        // TODO: check this code
        data->name = QString::fromUtf16((const ushort *)volumeInfo.iName.Ptr(), volumeInfo.iName.Length());
    }
}

void QDriveInfoPrivate::getFileSystemName()
{
    RFs &rfs = qt_s60GetRFs();

    TInt drive;
    if (RFs::CharToDrive(TChar(data->rootPath[0].toAscii()), drive) == KErrNone) {
        TFSName fileSystemName;
        if (rfs.FileSystemName(fileSystemName, drive) == KErrNone) {
            // TODO: check this code
            data->fileSystemName = QString::fromUtf16((const ushort *)fileSystemName.Ptr(), fileSystemName.Length());
        }
    }
}

void QDriveInfoPrivate::getRootPath()
{
    if (data->rootPath.isEmpty())
        return;

    // try to convert to X:/

    data->rootPath[0] = data->rootPath.at(0).toUpper();
    if (data->rootPath.at(0).unicode() >= 'A' && data->rootPath.at(0).unicode() <= 'Z') {
        if (data->rootPath.length() == 1) {
            data->rootPath.append(QLatin1String(":/"));
            return;
        }
        if (data->rootPath.length() >= 2 && data->rootPath.at(1) == QLatin1Char(':')) {
            if (data->rootPath.length() == 2) {
                data->rootPath.append(QLatin1Char('/'));
                return;
            }
            if (data->rootPath.at(2) == QLatin1Char('/'))
                return;
        }
    }

    data->rootPath.clear(); // invalid root path
}

static inline QDriveInfo::DriveType typeForDrive(const QString &rootPath)
{
    RFs &rfs = qt_s60GetRFs();

    TInt drive;
    if (RFs::CharToDrive(TChar(rootPath.at(0).toAscii()), drive) == KErrNone) {
        TDriveInfo driveInfo;
        if (rfs.Drive(driveInfo, drive) == KErrNone) {
            if (driveInfo.iType == EMediaRemote)
                return QDriveInfo::RemoteDrive;
            if (driveInfo.iType == EMediaCdRom)
                return QDriveInfo::CdromDrive;

            if (driveInfo.iDriveAtt & KDriveAttInternal)
                return QDriveInfo::InternalDrive;
            if (driveInfo.iDriveAtt & KDriveAttRemovable)
                return QDriveInfo::RemovableDrive;
        }
    }

    return QDriveInfo::InvalidDrive;
}

void QDriveInfoPrivate::getType()
{
    data->type = typeForDrive(data->rootPath);
}
