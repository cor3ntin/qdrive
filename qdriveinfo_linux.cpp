#include "qdriveinfo.h"
#include "qdriveinfo_p.h"

#include <mntent.h>
#include <sys/statfs.h>
#include <sys/stat.h>
#include <unistd.h>

#include <qplatformdefs.h>
#include <QtCore/QFile>
#include <QtCore/QDirIterator>
#include <QtCore/QTextStream>

#ifndef _PATH_MOUNTED
#  define _PATH_MOUNTED "/etc/mtab"
#endif
#ifndef _PATH_DISK_BY_LABEL
#  define _PATH_DISK_BY_LABEL "/dev/disk/by-label"
#endif

QList<QDriveInfo> QDriveInfoPrivate::drives()
{
    QList<QDriveInfo> drives;

    FILE *fp = setmntent(_PATH_MOUNTED, "r");
    if (fp) {
        struct mntent *mnt;
        while ((mnt = getmntent(fp)))
            drives.append(QDriveInfo(QString::fromLocal8Bit(mnt->mnt_dir)));
        endmntent(fp);
    }

    return drives;
}

void QDriveInfoPrivate::doStat(uint requiredFlags)
{
    if (data->getCachedFlag(requiredFlags))
        return;

    uint bitmask = 0;

    bitmask = CachedAvailableSizeFlag | CachedFreeSizeFlag | CachedSizeFlag |
              CachedValidFlag | CachedReadyFlag;
    if (requiredFlags & bitmask) {
        statFS();
        data->setCachedFlag(bitmask);
    }

    bitmask = this->CachedRootPathFlag | CachedFileSystemNameFlag | CachedDeviceFlag;
    if (requiredFlags & bitmask) {
        getMountEntry();
        data->setCachedFlag(bitmask);
    }

    bitmask = CachedTypeFlag;
    if (requiredFlags & bitmask) {
        getType();
        data->setCachedFlag(bitmask);
    }

    bitmask = CachedNameFlag;
    if (requiredFlags & bitmask) {
        getName();
        data->setCachedFlag(bitmask);
    }
}

void QDriveInfoPrivate::statFS()
{
    struct statfs statfs_buf;
    int result = statfs(data->rootPath.toUtf8().data(), &statfs_buf);
    if (result == -1) {
        data->valid = false;
        data->ready = false;
    } else {
        data->valid = true;
        data->ready = true;

        data->availableSize = statfs_buf.f_bavail * statfs_buf.f_bsize;
        data->freeSize = statfs_buf.f_bfree * statfs_buf.f_bsize;
        data->totalSize = statfs_buf.f_blocks * statfs_buf.f_bsize;
    }
}

void QDriveInfoPrivate::getMountEntry()
{
    FILE *fp = setmntent(_PATH_MOUNTED, "r");
    if (fp) {
        struct mntent *mnt;
        quint32 maxLength = 0;
        QString oldRootPath = data->rootPath;

        while ((mnt = getmntent(fp))) {
            QString mountDir = QString::fromLocal8Bit(mnt->mnt_dir);
            // we try to find most suitable entry
            if (oldRootPath.startsWith(mountDir) && maxLength < (quint32)mountDir.length()) {
                data->fileSystemName = QString::fromLatin1(mnt->mnt_type);
                data->device = QString::fromLocal8Bit(mnt->mnt_fsname);
                data->rootPath = mountDir;
                maxLength = mountDir.length();
            }
        }
        endmntent(fp);
    }
}

static inline QDriveInfo::DriveType determineType(const QString &device)
{
    QString dmFile;

    if (device.contains(QLatin1String("mapper"))) {
        QT_STATBUF stat_buf;
        QT_STAT(device.toLocal8Bit(), &stat_buf);

        dmFile = QLatin1String("dm-") + QString::number(stat_buf.st_rdev & 0377);
    } else {
        dmFile = device.section(QLatin1Char('/'), 2, 3);
        if (dmFile.startsWith(QLatin1String("mmc"))) {
            // assume this dev is removable sd/mmc card.
            return QDriveInfo::RemovableDrive;
        }

        if (dmFile.length() > 3) {
            // if device has number, we need the 'parent' device
            dmFile.chop(1);
            if (dmFile.right(1) == QLatin1String("p")) // get rid of partition number
                dmFile.chop(1);
        }
    }
    dmFile = QLatin1String("/sys/block/") + dmFile + QLatin1String("/removable");

    QFile file(dmFile);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream sysinfo(&file);
        QString line = sysinfo.readAll();
        if (line.contains(QLatin1Char('1')))
            return QDriveInfo::RemovableDrive;
    }

    if (device.startsWith(QLatin1String("/dev")))
        return QDriveInfo::InternalDrive;

    return QDriveInfo::InvalidDrive;
}

void QDriveInfoPrivate::getType()
{
    // we need a device and filesystem name to get info
    doStat(CachedDeviceFlag | CachedFileSystemNameFlag);

    data->type = determineType(data->device);
    if (data->type == QDriveInfo::InvalidDrive) {
        // test for UNC shares
        if (data->rootPath.startsWith(QLatin1String("//")) ||
            data->fileSystemName == QLatin1String("nfs")) {
            data->type == QDriveInfo::RemoteDrive;
        }
    }
}

// we need udev to be present in system to get label name
// Unfortunately, i don't know proper way to get labels except this. Maybe libudev can provide
// this information. TODO: explore it
// If we have udev installed in the system, proper symlinks are created by it, so we don't
// need to link to libudev
// If not, i don't know other way to get labels without root privelegies
void QDriveInfoPrivate::getName()
{
    QFileInfo fi(QLatin1String(_PATH_DISK_BY_LABEL));
    if (!fi.exists() || !fi.isDir()) // "/dev/disk/by-label" doesn't exists or invalid
        return;

    doStat(CachedDeviceFlag); // we need device to get info

    QDirIterator it(QLatin1String(_PATH_DISK_BY_LABEL), QDir::NoDotAndDotDot);
    while (it.hasNext()) {
        it.next();
        QFileInfo fileInfo(it.filePath());
        if (fileInfo.isSymLink() && data->device == fileInfo.symLinkTarget()) {
            data->name = fileInfo.fileName();
            break;
        }
    }
}
