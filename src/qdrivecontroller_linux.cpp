#include "qdrivecontroller.h"
#include "qdrivecontroller_p.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QProcess>
#include <QtCore/QSet>
#include <QtCore/QSocketNotifier>
#include <QtCore/QTimer>

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

#include  "qdriveinfo.h"

#ifndef _PATH_MOUNTED
#  define _PATH_MOUNTED "/etc/mtab"
#endif

class QDriveWatcherEngine : public QObject
{
    Q_OBJECT

public:
    QDriveWatcherEngine(QObject *parent);
    ~QDriveWatcherEngine();

    inline bool isValid() const
    { return mtabWatchA > 0; }

Q_SIGNALS:
    void driveAdded(const QString &path);
    void driveRemoved(const QString &path);

private Q_SLOTS:
    void deviceChanged();
    void inotifyActivated();

private:
    QSet<QString> drives;
    int inotifyFD;
    int mtabWatchA;
};

static QSet<QString> getDrives()
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


QDriveWatcherEngine::QDriveWatcherEngine(QObject *parent)
    : QObject(parent)
{
    drives = getDrives();
    inotifyFD = ::inotify_init();
    mtabWatchA = ::inotify_add_watch(inotifyFD, _PATH_MOUNTED, IN_MODIFY);
    if (mtabWatchA > 0) {
        QSocketNotifier *notifier = new QSocketNotifier(inotifyFD, QSocketNotifier::Read, this);
        connect(notifier, SIGNAL(activated(int)), this, SLOT(inotifyActivated()));
    }
}

QDriveWatcherEngine::~QDriveWatcherEngine()
{
    ::close(inotifyFD);
}

void QDriveWatcherEngine::deviceChanged()
{
    QSet<QString> allNewDrives = getDrives();

    foreach (const QString &drive, allNewDrives) {
        if (!drives.contains(drive))
            emit driveAdded(drive);
    }

    foreach (const QString &drive, drives) {
        if (!allNewDrives.contains(drive))
            emit driveRemoved(drive);
    }

    drives = allNewDrives;
}

void QDriveWatcherEngine::inotifyActivated()
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


bool QDriveWatcher::start_sys()
{
    engine = new QDriveWatcherEngine(this);
    connect(engine, SIGNAL(driveAdded(QString)), this, SIGNAL(driveAdded(QString)));
    connect(engine, SIGNAL(driveRemoved(QString)), this, SIGNAL(driveRemoved(QString)));
    return engine->isValid();
}

void QDriveWatcher::stop_sys()
{
    delete engine;
    engine = 0;
}

static bool mountUdisks(const QString &device, QString &mount_point, QString &status, const QString &fs, const QString &options)
{
    QProcess mount;
    QString command = "udisks";
    QStringList args;

    mount.setProcessChannelMode(QProcess::MergedChannels);

    args << "--mount";

    if (!fs.isEmpty()) {
        args << "--mount-fstype" << fs;
    }

    if (!options.isEmpty()) {
        args << "--mount-options" << options;
    }

    args << device;

    mount.start(command, args);
    if (!mount.waitForStarted()) {
        status = "Trouble with mount: start issue";
        return false;
    }

    if (!mount.waitForFinished()) {
        status = "Trouble with mount: finish issue";
        return false;
    }

    //int code       = mount.exitCode();
    bool is_ok     = false;
    QString buffer = mount.readAll();


    // Stupid 'udisks' in any cases return 0 exit status!
    is_ok = !buffer.contains(QRegExp("^Mount failed:", Qt::CaseInsensitive));

    if (is_ok) {
        QStringList list = buffer.trimmed().split(" ", QString::SkipEmptyParts);
        if (list.count() == 4)
            mount_point = list.at(3);
        status = "Ok";
    } else {
        status = buffer;
    }

    return is_ok;
}

static bool unmountUdisks(const QString &device, QString &status)
{
    QProcess unmount;
    QString command = "udisks";
    QStringList args;

    unmount.setProcessChannelMode(QProcess::MergedChannels);

    args << "--unmount" << device;

    unmount.start(command, args);
    if (!unmount.waitForStarted()) {
        status = "Trouble with unmount: start issue";
        return false;
    }

    if (!unmount.waitForFinished()) {
        status = "Trouble with unmount: finish issue";
        return false;
    }

    //int code       = unmount.exitCode();
    bool is_ok = false;
    QString buffer = unmount.readAll();

    // Stupid 'udisks' in any cases return 0 exit status!
    is_ok = !buffer.contains(QRegExp("^Unmount failed:", Qt::CaseInsensitive));

    if (is_ok)
        status = "Ok";
    else
        status = buffer;

    return is_ok;
}

bool QDriveController::mount(const QString &device, const QString &path)
{
    QString status;
    QString mountPath = path;
    bool result = mountUdisks(device, mountPath, status, QString(), QString());
    if (!result) {
        qWarning() << "error mounting" << status;
    }
    return result;
}

bool QDriveController::unmount(const QString &path)
{
    QString status;
    bool result = unmountUdisks(QDriveInfo(path).device(), status);
    if (!result) {
        qWarning() << "error mounting" << status;
    }
    return result;
}

bool QDriveController::eject(const QString &device)
{
    if (!unmount(device)) {
        qDebug() << "failed to unmount";
        return false;
    }

    int fd = ::open(QFile::encodeName(device), O_RDONLY | O_NONBLOCK);
    if (fd == -1) {
        d->setLastError(errno);
        return false;
    }

//#if defined(CDROMEJECT)
//	status = ioctl(fd, CDROMEJECT);
//#elif defined(CDIOCEJECT)
//	status = ioctl(fd, CDIOCEJECT);
//#else

    int result = ::ioctl(fd, CDROMEJECT);
    if (result == -1) {
        d->setLastError(errno);
        close(fd);
        return false;
    }
    close(fd);
    return true;
}

#include "qdrivecontroller_linux.moc"
