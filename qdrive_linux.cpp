#include "qdrive_linux_p.h"

#include <mntent.h>
#include <sys/statfs.h>
#include <sys/stat.h>
#include <unistd.h>

#include <QtCore/QFile>
#include <QDebug>

//================================== QDriveInfoPrivate ==================================

QDrivePrivate::QDrivePrivate()
{
}

void QDrivePrivate::stat(uint requiredFlags)
{
    uint bitmask = 0;
    if (requiredFlags & CachedReadyFlag) {
        ready = true;
        setCachedFlag(CachedReadyFlag);
    }

    bitmask = CachedAvailableSizeFlag | CachedFreeSizeFlag | CachedSizeFlag;
    if (requiredFlags & bitmask &&
        !getCachedFlag(bitmask))
        statFS();

    bitmask = CachedFileSystemNameFlag | CachedDeviceFlag;
    if (requiredFlags & bitmask &&
        !getCachedFlag(bitmask))
        getMountEntry();

    bitmask = CachedTypeFlag;
    if (requiredFlags & bitmask &&
        !getCachedFlag(bitmask))
        getType();
}

void QDrivePrivate::statFS()
{
    struct statfs statFS;
    statfs(rootPath.toUtf8().data(), &statFS);

    size = statFS.f_blocks*statFS.f_bsize;
    freeSize = statFS.f_bfree*statFS.f_bsize;
    availableSize = statFS.f_bavail*statFS.f_bsize;

    setCachedFlag(CachedAvailableSizeFlag |
                  CachedFreeSizeFlag |
                  CachedSizeFlag);
}

void QDrivePrivate::getMountEntry()
{
    struct mntent *mnt;
    const char *table = MOUNTED;
    FILE *fp;

    fp = setmntent (table, "r");
    if (fp == NULL)
        return;

    while ((mnt = getmntent (fp)))
    {
        if (mnt->mnt_dir == rootPath) {
            fileSystemName = QString(mnt->mnt_type);
            device = QString(mnt->mnt_fsname);
            break;
        }
    }

    endmntent(fp);

    setCachedFlag(CachedFileSystemNameFlag | CachedDeviceFlag);
}

void QDrivePrivate::getType()
{
    stat(CachedDeviceFlag); // we need a device to get info

    type = determineType();

    setCachedFlag(CachedTypeFlag);
}

// From Qt Mobility
QDrive::DriveType QDrivePrivate::determineType()
{
    QString dmFile;

//    if(mountEntriesMap.value(driveVolume).contains("mapper")) {
    if(device.contains("mapper")) {
        struct stat stat_buf;
        ::stat(device.toLatin1(), &stat_buf);

        dmFile = QString("/sys/block/dm-%1/removable").arg(stat_buf.st_rdev & 0377);

    } else {

        dmFile = device.section("/",2,3);
        if (dmFile.left(3) == "mmc") { //assume this dev is removable sd/mmc card.
            return QDrive::RemovableDrive;
        }

        if(dmFile.length() > 3) { //if device has number, we need the 'parent' device
            dmFile.chop(1);
            if (dmFile.right(1) == "p") //get rid of partition number
                dmFile.chop(1);
        }
        dmFile = "/sys/block/"+dmFile+"/removable";
    }

    QFile file(dmFile);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open sys file";
    } else {
        QTextStream sysinfo(&file);
        QString line = sysinfo.readAll();
        if(line.contains("1")) {
            return QDrive::RemovableDrive;
        }
    }

    if(rootPath.left(2) == "//") {
        return QDrive::RemoteDrive;
    }
    if (device.startsWith("/dev"))
        return QDrive::InternalDrive;
    else
        return QDrive::NoDrive;
}

QStringList QDrive::drivePaths()
{
    QStringList ret;
    struct mntent *mnt;
    const char *table = MOUNTED;
    FILE *fp;

    fp = setmntent (table, "r");
    if (fp == NULL)
        return ret;

    while ((mnt = getmntent (fp)))
    {
//        me = xmalloc (sizeof *me);
        ret.append(QString(mnt->mnt_dir));
//        me->me_devname = xstrdup (mnt->mnt_fsname);
//        me->me_mountdir = xstrdup (mnt->mnt_dir);
//        me->me_type = xstrdup (mnt->mnt_type);
//        me->me_type_malloced = 1;
//        me->me_dummy = ME_DUMMY (me->me_devname, me->me_type);
//        me->me_remote = ME_REMOTE (me->me_devname, me->me_type);
//        me->me_dev = dev_from_mount_options (mnt->mnt_opts);

//        /* Add to the linked list. */
//        *mtail = me;
//        mtail = &me->me_next;
    }

    endmntent(fp);

    return ret;
}
