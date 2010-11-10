#include "qdriveinfo.h"
#include "qdriveinfo_p.h"

#include <mntent.h>
#include <sys/statfs.h>
#include <sys/stat.h>
#include <unistd.h>

#include <QtCore/QFile>
#include <QtCore/QDirIterator>

#ifndef _PATH_MOUNTED
#  define _PATH_MOUNTED "/etc/mtab"
#endif
#ifndef _PATH_DISK_BY_LABEL
#  define _PATH_DISK_BY_LABEL "/dev/disk/by-label"
#endif

QStringList drivePaths()
{
    QStringList ret;

    FILE *fp = setmntent(_PATH_MOUNTED, "r");
    if (!fp)
        return ret;

    struct mntent *mnt;
    while ((mnt = getmntent(fp)))
        ret.append(QString::fromLocal8Bit(mnt->mnt_dir));

    endmntent(fp);

    return ret;
}

QList<QDriveInfo> QDriveInfoPrivate::drives()
{
    QList<QDriveInfo> result;
    foreach (const QString &path, drivePaths())
        result.append(QDriveInfo(path));
    return result;
}

void QDriveInfoPrivate::stat(uint requiredFlags)
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

    bitmask = CachedFileSystemNameFlag | CachedDeviceFlag;
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
    struct statfs statFS;
    int result = statfs(data->rootPath.toUtf8().data(), &statFS);
    if (result == -1) {
        data->valid = false;
        data->ready = false;
    } else {
        data->availableSize = statFS.f_bavail * statFS.f_bsize;
        data->freeSize = statFS.f_bfree * statFS.f_bsize;
        data->totalSize = statFS.f_blocks * statFS.f_bsize;

        data->valid = true;
        data->ready = true;
    }
}

void QDriveInfoPrivate::getMountEntry()
{
    FILE *fp = setmntent(_PATH_MOUNTED, "r");
    if (!fp)
        return;

    struct mntent *mnt;
    while ((mnt = getmntent(fp))) {
        if (mnt->mnt_dir == data->rootPath) {
            // we found our entry
            data->fileSystemName = QString::fromLatin1(mnt->mnt_type);
            data->device = QString::fromLocal8Bit(mnt->mnt_fsname);
            break;
        }
    }

    endmntent(fp);
}

static inline QDriveInfo::DriveType determineType(const QString &device)
{
    QString dmFile;

    if (device.contains(QLatin1String("mapper"))) {
        struct stat stat_buf;
        ::stat(device.toLocal8Bit(), &stat_buf);

        dmFile = QString::number(stat_buf.st_rdev & 0377);
        dmFile = QLatin1String("/sys/block/dm-") + dmFile + QLatin1String("/removable");
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
        dmFile = QLatin1String("/sys/block/") + dmFile + QLatin1String("/removable");
    }

    QFile file(dmFile);
    if (!file.open(QIODevice::ReadOnly)) {
//        qWarning() << "Could not open sys file";
    } else {
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
    stat(CachedDeviceFlag); // we need a device to get info

    data->type = determineType(data->device);
    if (data->type == QDriveInfo::InvalidDrive) {
        // test for UNC shares
        if (data->rootPath.startsWith(QLatin1String("//")))
            data->type == QDriveInfo::RemoteDrive;
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
    if (!fi.exists() && !fi.isDir()) // /dev/disk/by-label doesn't exists or invalid
        return;

    stat(CachedDeviceFlag); // we need device to get info

    QDirIterator it(QLatin1String(_PATH_DISK_BY_LABEL), QDir::NoDotAndDotDot);
    while (it.hasNext()) {
        QString entry = it.next();
        QFileInfo fileInfo(entry);
        if (fileInfo.isSymLink()) {
            // ok, it is symlink to device and we work with it
            QString target = fileInfo.symLinkTarget();
            if (data->device == target) {
                data->name = fileInfo.fileName();
                break;
            }
        }
    }
}
