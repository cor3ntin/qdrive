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

QDriveWatcher::QDriveWatcher(QObject *parent) :
        QObject(parent)
{
    qDebug("QDriveWatcher::QDriveWatcher");
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
    QSet<QString> allNewDrives;

    FILE *fp = ::setmntent(_PATH_MOUNTED, "r");
    if (fp) {
        struct mntent *mnt;
        while ((mnt = ::getmntent(fp))) {
            QString rootPath = QFile::decodeName(mnt->mnt_dir);
            if (!allNewDrives.contains(rootPath)) {
                allNewDrives.insert(rootPath);
            }
        }
        ::endmntent(fp);
    }

    QSet<QString> newDrives = newDrives.subtract(drives);
    QSet<QString> oldDrives = drives.subtract(allNewDrives);

    foreach (QString drive, newDrives) {
        emit driveAdded(drive);
    }

    foreach (QString drive, oldDrives) {
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
