#include "qdrivecontroller_p.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTimer>
#include <QtCore/QSet>
#include <QtCore/QSocketNotifier>


#include <QDebug>

#include <sys/inotify.h>
#include <sys/mount.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <mntent.h>
#include <linux/cdrom.h>

#ifndef _PATH_MOUNTED
#  define _PATH_MOUNTED "/etc/mtab"
#endif

QSet<QString> getDrives()
{
    QSet<QString> result;

    FILE *fp = ::setmntent(_PATH_MOUNTED, "r");
    if (fp) {
        struct mntent *mnt;
        while ((mnt = ::getmntent(fp))) {
            QString rootPath = QFile::decodeName(mnt->mnt_dir);
            if (!result.contains(rootPath)) {
                result.insert(rootPath);
            }
        }
        ::endmntent(fp);
    }

    return result;
}

QDriveWatcher::QDriveWatcher(QObject *parent) :
        QObject(parent)
{
    drives = getDrives();
    inotifyFD = ::inotify_init();
    mtabWatchA = ::inotify_add_watch(inotifyFD, _PATH_MOUNTED, IN_MODIFY);
    if (mtabWatchA > 0) {
        QSocketNotifier *notifier = new QSocketNotifier(inotifyFD, QSocketNotifier::Read, this);
        connect(notifier, SIGNAL(activated(int)), this, SLOT(inotifyActivated()));
    }
}

QDriveWatcher::~QDriveWatcher()
{
    ::close(inotifyFD);
}

void QDriveWatcher::deviceChanged()
{
    QSet<QString> allNewDrives = getDrives();

    foreach (QString drive, allNewDrives) {
        if (!drives.contains(drive))
            emit driveAdded(drive);
    }

    foreach (QString drive, drives) {
        if (!allNewDrives.contains(drive))
            emit driveRemoved(drive);
    }

    drives = allNewDrives;
}

void QDriveWatcher::inotifyActivated()
{
    char buffer[1024];
    struct inotify_event *event;
    int len = ::read(inotifyFD, (void *)buffer, sizeof(buffer));
    if (len > 0) {
        event = (struct inotify_event *)buffer;
        if (event->wd == mtabWatchA /*&& (event->mask & IN_IGNORED) == 0*/) {
            ::inotify_rm_watch(inotifyFD, mtabWatchA);

            QTimer::singleShot(1000, this, SLOT(deviceChanged())); //give this time to finish write

            mtabWatchA = ::inotify_add_watch(inotifyFD, _PATH_MOUNTED, IN_MODIFY);
        }
    }
}

bool QDriveController::mount(const QString &device, const QString &path)
{
    QString targetPath = QDir::toNativeSeparators(path);
    if (!targetPath.endsWith('/'))
        targetPath.append('/');

    // TODO: guess filesystem
    int result = ::mount(QFile::encodeName(device), QFile::encodeName(targetPath), "ext4", 0, 0);
    if (result) {
        qDebug() << "mount failed" << result << errno << QString::fromLocal8Bit(::strerror(errno));
        return false;
    }
    return true;
}

bool QDriveController::unmount(const QString &path)
{
    QString targetPath = QDir::toNativeSeparators(path);
    if (!targetPath.endsWith('/'))
        targetPath.append('/');

    int result = ::umount(QFile::encodeName(targetPath));
    if (result) {
        qDebug() << "unmount failed" << result << errno << QString::fromLocal8Bit(::strerror(errno));
        return false;
    }
    return true;
}

bool QDriveController::eject(const QString &device)
{
// TODO: unmount ?
//    if (!unmount(device)) {
//        qDebug() << "failed to unmount";
//        return false;
//    }

    int fd = ::open(QFile::encodeName(device), O_RDONLY | O_NONBLOCK);
    if (fd == -1) {
        qDebug() << "can't open device" << errno << QString::fromLocal8Bit(::strerror(errno));
        return false;
    }

//#if defined(CDROMEJECT)
//	status = ioctl(fd, CDROMEJECT);
//#elif defined(CDIOCEJECT)
//	status = ioctl(fd, CDIOCEJECT);
//#else

    int result = ::ioctl(fd, CDROMEJECT);
    if (result == -1) {
        qDebug() << "can't open device" << result << errno << QString::fromLocal8Bit(::strerror(errno));
        close(fd);
        return false;
    }
    close(fd);
    return true;
}
