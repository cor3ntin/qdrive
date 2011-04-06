#include "qdrivecontroller_p.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QSocketNotifier>
#include <QtCore/QTimer>
#include <QtCore/QFile>

#include <sys/stat.h>
//#include <sys/vfs.h>
#include <sys/inotify.h>
#include <mntent.h>

#include <QDebug>

INotifyWatcher *INotifyWatcher::watcher = 0;

INotifyWatcher::INotifyWatcher(QObject *parent) :
        QObject(parent)
{
    qDebug("INotifyWatcher::INotifyWatcher");
    inotifyFD = ::inotify_init();
    mtabWatchA = ::inotify_add_watch(inotifyFD, "/etc/mtab", IN_MODIFY);
    if (mtabWatchA > 0) {
        QSocketNotifier *notifier = new QSocketNotifier(inotifyFD, QSocketNotifier::Read, this);
        connect(notifier, SIGNAL(activated(int)), this, SLOT(inotifyActivated()));
    }
}

INotifyWatcher::~INotifyWatcher()
{
    ::close(inotifyFD);
}

void INotifyWatcher::deviceChanged()
{
    FILE *fp = ::setmntent(_PATH_MOUNTED, "r");
    if (fp) {
        struct mntent *mnt;
        while ((mnt = ::getmntent(fp))) {
            QString rootPath = QFile::decodeName(mnt->mnt_dir);
            if (!drivePaths.contains(rootPath)) {
                drivePaths.insert(rootPath);
                testMount(rootPath);
            }
        }
        ::endmntent(fp);
    }

}

void INotifyWatcher::inotifyActivated()
{
    qDebug("INotifyWatcher::inotifyActivated");

    char buffer[1024];
    struct inotify_event *event;
    int len = ::read(inotifyFD, (void *)buffer, sizeof(buffer));
    if (len > 0) {
        event = (struct inotify_event *)buffer;
        if (event->wd == mtabWatchA /*&& (event->mask & IN_IGNORED) == 0*/) {
            ::inotify_rm_watch(inotifyFD, mtabWatchA);
            QTimer::singleShot(1000,this,SLOT(deviceChanged()));//give this time to finish write
            mtabWatchA = ::inotify_add_watch(inotifyFD, "/etc/mtab", IN_MODIFY);
        }
    }
}

QDriveControllerPrivate::QDriveControllerPrivate()
{
    if (!INotifyWatcher::watcher) {
        // TODO: add mutex
        INotifyWatcher::watcher = new INotifyWatcher(qApp);
    }
}

QDriveControllerPrivate::~QDriveControllerPrivate()
{

}

void INotifyWatcher::testMount(const QString &path)
{
    qDebug() << "INotifyWatcher::testMount" << path;
}
