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
//    qDebug() << "mounting" << c.mount("/dev/sdd", QString());
}

void Monitor::testUnmount()
{
    QDriveController c;
//    qDebug() << "unmounting" << c.unmount("/dev/sdd");
}
