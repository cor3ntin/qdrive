#include "qdrivecontroller_p.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QSocketNotifier>
#include <QtCore/QTimer>
#include <QtCore/QFile>
#include <QtCore/QSet>

#include <sys/stat.h>
#include <sys/inotify.h>
#include <mntent.h>

#include <QDebug>

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
    qDebug("QDriveWatcher::QDriveWatcher");
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
    qDebug("QDriveWatcher::inotifyActivated");

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
