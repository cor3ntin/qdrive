#include "qdrive_linux_p.h"

#include <sys/statfs.h>
#include <mntent.h>
//#include <sysfs.h>
//#include <syscall.h>
//#include <unistd.h>
#include <linux/unistd.h>
#include <linux/kernel.h>
//#include <sys/fstyp.h>
//#include <sys/fsid.h>

#include <QDebug>

//================================== QDriveInfoPrivate ==================================

//extern "C" {
//int sysfs(int option, unsigned int fs_index, char *buf);
//    int sysfs(int, ...);
//}

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

    char buf[255];
//    int error = ::sysfs(2, statFS.f_type, buf);
}

void QDrivePrivate::getMountEntry()
{
    struct mntent *mnt;
    char *table = MOUNTED;
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

QStringList QDrive::drivePaths()
{
    QStringList ret;
    struct mntent *mnt;
    char *table = MOUNTED;
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
