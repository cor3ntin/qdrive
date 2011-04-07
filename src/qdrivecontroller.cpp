#include "qdrivecontroller.h"
#include "qdrivecontroller_p.h"

#include <QtCore/QCoreApplication>

#include <QDebug>

QDriveWatcher *QDriveControllerPrivate::watcherInstance = 0;

QDriveControllerPrivate::QDriveControllerPrivate()
{
    if (!watcherInstance) {
        // TODO: add mutexes
        watcherInstance = new QDriveWatcher(qApp);
    }
}

QDriveControllerPrivate::~QDriveControllerPrivate()
{
}

QDriveController::QDriveController(QObject *parent) :
    QObject(parent),
    d_ptr(new QDriveControllerPrivate)
{
    connect(QDriveControllerPrivate::watcherInstance, SIGNAL(driveAdded(QString)),
            this, SIGNAL(driveMounted(QString)));
    connect(QDriveControllerPrivate::watcherInstance, SIGNAL(driveRemoved(QString)),
            this, SIGNAL(driveUnmounted(QString)));

    // TODO: remove in release
    connect(this, SIGNAL(driveMounted(QString)), SLOT(testDriveMounted(QString)));
    connect(this, SIGNAL(driveUnmounted(QString)), SLOT(testDriveUnounted(QString)));
}

void QDriveController::testDriveMounted(const QString &path)
{
    qDebug() << "We got new drive! Mounted at" << path;
}

void QDriveController::testDriveUnounted(const QString &path)
{
    qDebug() << "We lost drive! Was mounted at" << path;
}


