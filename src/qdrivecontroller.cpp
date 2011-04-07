#include "qdrivecontroller.h"
#include "qdrivecontroller_p.h"

#include <QtCore/QCoreApplication>

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
    QObject::connect(QDriveControllerPrivate::watcherInstance, SIGNAL(driveAdded(QString)),
                     this, SIGNAL(driveMounted(QString)));
    QObject::connect(QDriveControllerPrivate::watcherInstance, SIGNAL(driveRemoved(QString)),
                     this, SIGNAL(driveUnmounted(QString)));
}


