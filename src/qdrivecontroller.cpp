#include "qdrivecontroller.h"
#include "qdrivecontroller_p.h"

#include <QtCore/QCoreApplication>

QDriveWatcher::QDriveWatcher(QObject *parent)
    : QObject(parent),
      startStopCounter(0), engine(0)
{
}

QDriveWatcher::~QDriveWatcher()
{
    if (startStopCounter != 0)
        qWarning("QDriveWatcher is going to be deleted but it seems like it is still in use.");

    stop_sys();
}

void QDriveWatcher::start()
{
    startStopCounter.ref();
    if (startStopCounter == 1) {
        if (!start_sys())
            stop();
    }
}

void QDriveWatcher::stop()
{
    if (!startStopCounter.deref())
        stop_sys();
}

Q_GLOBAL_STATIC(QDriveWatcher, theWatcher);


QDriveController::QDriveController(QObject *parent)
    : QObject(parent)
{
    if (QDriveWatcher *watcher = theWatcher()) {
        connect(watcher, SIGNAL(driveAdded(QString)),
                this, SIGNAL(driveMounted(QString)), Qt::QueuedConnection);
        connect(watcher, SIGNAL(driveRemoved(QString)),
                this, SIGNAL(driveUnmounted(QString)), Qt::QueuedConnection);

        watcher->start();
    }
    // ### removeme!
    connect(this, SIGNAL(driveMounted(QString)), SLOT(testDriveMounted(QString)));
    connect(this, SIGNAL(driveUnmounted(QString)), SLOT(testDriveUnounted(QString)));
    //
}

QDriveController::~QDriveController()
{
    if (QDriveWatcher *watcher = theWatcher())
        watcher->stop();
}
// ### removeme!
#include <QDebug>
void QDriveController::testDriveMounted(const QString &path)
{
    qDebug() << "We got new drive! Mounted at" << path;
}

void QDriveController::testDriveUnounted(const QString &path)
{
    qDebug() << "We lost drive! Was mounted at" << path;
}
//
