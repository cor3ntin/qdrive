#include "qdriveinfo_p.h"

#include <QtCore/QDirIterator>
#include <QtCore/QTextStream>

#include <mntent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/vfs.h>

#ifndef _PATH_MOUNTED
#  define _PATH_MOUNTED "/etc/mtab"
#endif
#ifndef _PATH_DISK_BY_LABEL
#  define _PATH_DISK_BY_LABEL "/dev/disk/by-label"
#endif

void QDriveInfoPrivate::initRootPath()
{
    if (data->rootPath.isEmpty())
        return;

    FILE *fp = ::setmntent(_PATH_MOUNTED, "r");
    if (fp) {
        QString oldRootPath = data->rootPath;
        quint32 maxLength = 0;

        struct mntent *mnt;
        while ((mnt = ::getmntent(fp))) {
            QString mountDir = QFile::decodeName(mnt->mnt_dir);
            // we try to find most suitable entry
            if (oldRootPath.startsWith(mountDir) && maxLength < (quint32)mountDir.length()) {
                data->fileSystemName = QString::fromLatin1(mnt->mnt_type);
                data->device = QFile::decodeName(mnt->mnt_fsname);
                data->rootPath = mountDir;
                maxLength = mountDir.length();
            }
        }
        ::endmntent(fp);
    }
}

static inline QDriveInfo::DriveType determineType(const QString &device)
{
    QString dmFile;

    if (device.contains(QLatin1String("mapper"))) {
        QT_STATBUF stat_buf;
        QT_STAT(QFile::encodeName(device).constData(), &stat_buf);

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
            if (dmFile.at(dmFile.length() - 1) == QLatin1Char('p')) // get rid of partition number
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

// we need udev to be present in system to get label name
// Unfortunately, i don't know proper way to get labels except this. Maybe libudev can provide
// this information. TODO: explore it
// If we have udev installed in the system, proper symlinks are created by it, so we don't
// need to link to libudev
// If not, i don't know other way to get labels without root privelegies
static inline QString getName(const QString &device)
{
    QDirIterator it(QLatin1String(_PATH_DISK_BY_LABEL), QDir::NoDotAndDotDot);
    while (it.hasNext()) {
        it.next();
        QFileInfo fileInfo(it.fileInfo());
        if (fileInfo.isSymLink() && fileInfo.symLinkTarget() == device)
            return fileInfo.fileName();
    }

    return QString();
}

void QDriveInfoPrivate::doStat(uint requiredFlags)
{
    if (data->getCachedFlag(requiredFlags))
        return;

    if (!data->getCachedFlag(CachedRootPathFlag)) {
        initRootPath();
        data->setCachedFlag(CachedRootPathFlag | CachedFileSystemNameFlag | CachedDeviceFlag);
    }

    if (data->rootPath.isEmpty() || (data->getCachedFlag(CachedValidFlag) && !data->valid))
        return;

    if (!data->getCachedFlag(CachedValidFlag))
        requiredFlags |= CachedValidFlag; // force drive validation


    uint bitmask = 0;

    bitmask = CachedTotalSizeFlag | CachedFreeSizeFlag | CachedAvailableSizeFlag |
              CachedCapabilitiesFlag | CachedReadyFlag | CachedValidFlag;
    if (requiredFlags & bitmask) {
        getVolumeInfo();
        data->setCachedFlag(bitmask);

        if (!data->valid)
            return;
    }

    bitmask = CachedNameFlag;
    if (requiredFlags & bitmask) {
        data->name = getName(data->device);
        data->setCachedFlag(bitmask);
    }

    bitmask = CachedTypeFlag;
    if (requiredFlags & bitmask) {
        data->type = determineType(data->device);
        if (data->type == QDriveInfo::InvalidDrive) {
            // test for UNC shares
            if (data->rootPath.startsWith(QLatin1String("//"))
                || data->fileSystemName.toLower() == QLatin1String("nfs")) {
                data->type = QDriveInfo::RemoteDrive;
            }
        }
        data->setCachedFlag(bitmask);
    }
}

void QDriveInfoPrivate::getVolumeInfo()
{
    struct statfs statfs_buf;
    int result;
    do {
        result = ::statfs(QFile::encodeName(data->rootPath).constData(), &statfs_buf);
    } while (result != 0 && errno == EINTR);
    if (result == 0) {
        data->valid = true;
        data->ready = true;

        data->totalSize = statfs_buf.f_blocks * statfs_buf.f_bsize;
        data->freeSize = statfs_buf.f_bfree * statfs_buf.f_bsize;
        data->availableSize = statfs_buf.f_bavail * statfs_buf.f_bsize;

        if (statfs_buf.f_flag & ST_RDONLY)
            data->capabilities |= QDriveInfo::ReadOnlyVolume;

        // ### check if an alternative way exists
        QString fsName = data->fileSystemName.toLower();
        if (!fsName.startsWith(QLatin1String("fat"))
            && fsName != QLatin1String("hfs") && fsName != QLatin1String("hpfs")) {
            if (!fsName.startsWith(QLatin1String("reiser"))
                && !fsName.contains(QLatin1String("9660")) && !fsName.contains(QLatin1String("joliet"))) {
                data->capabilities |= QDriveInfo::AccessControlListsSupport;
            }
            data->capabilities |= QDriveInfo::CaseSensitiveFileNames;
            data->capabilities |= QDriveInfo::HardlinksSupport;
            data->capabilities |= QDriveInfo::SymlinksSupport;
        }
    }
}

QList<QDriveInfo> QDriveInfoPrivate::drives()
{
    QList<QDriveInfo> drives;

    FILE *fp = ::setmntent(_PATH_MOUNTED, "r");
    if (fp) {
        struct mntent *mnt;
        while ((mnt = ::getmntent(fp)))
            drives.append(QDriveInfo(QFile::decodeName(mnt->mnt_dir)));
        ::endmntent(fp);
    }

    return drives;
}
