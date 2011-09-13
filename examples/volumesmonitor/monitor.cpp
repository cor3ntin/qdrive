#include "monitor.h"

#include <QtCore/QDebug>

#include <QDriveController>

Monitor::Monitor(QDriveController *controller) : QObject(controller)
{
    connect(controller, SIGNAL(driveMounted(QString)), this, SLOT(driveMounted(QString)));
    connect(controller, SIGNAL(driveUnmounted(QString)), this, SLOT(driveUnmounted(QString)));
}

Monitor::~Monitor()
{
}

void Monitor::driveMounted(const QString &path)
{
    qDebug() << "We got new drive! Mounted at" << path;
}

void Monitor::driveUnmounted(const QString &path)
{
    qDebug() << "We lost a drive! Was mounted at" << path;
}

void Monitor::testMount()
{
    QDriveController c;
    bool result = true;
//    result = c.mount("/dev/sdd", QString());
    if (result)
        qDebug() << "Mounting succeeded";
    else
        qDebug() << "Mounting failed:" << c.errorString();
}

void Monitor::testUnmount()
{
    QDriveController c;
    bool result = true;
//    result = c.unmount("/dev/sdd");

    if (result)
        qDebug() << "Unmounting succeeded";
    else
        qDebug() << "Unmounting failed:" << c.errorString();
}
