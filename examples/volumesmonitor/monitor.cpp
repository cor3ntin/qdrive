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
#if defined(Q_OS_MAC)
//    result = c.mount("/dev/disk1");
#elif defined(Q_OS_WIN)
//    result = c.mount("\\\\vmware-host\\Shared Folders\\arch", "Z:");
#else
//    result = c.mount("/dev/sdd", QString());
#endif

    if (result)
        qDebug() << "Mounting succeeded";
    else
        qDebug() << "Mounting failed:" << c.errorString();
}

void Monitor::testUnmount()
{
    QDriveController c;
    bool result = true;
#ifdef Q_OS_MAC
    result = c.unmount("/Volumes/NO NAME"); // ok
//    result = c.unmount("/dev/disk1"); // fail
#elif defined(Q_OS_WIN)
//    result = c.unmount("Z:/"); // ok
#else
//    result = c.unmount("/dev/sdd");
#endif

    if (result)
        qDebug() << "Unmounting succeeded";
    else
        qDebug() << "Unmounting failed:" << c.errorString();
}

void Monitor::testEject()
{
    QDriveController c;
    bool result = true;
#ifdef Q_OS_MAC
    result = c.eject("/Volumes/NO NAME"); // ok
//    result = c.eject("/dev/disk1"); // fail
#elif defined(Q_OS_WIN)
    result = c.eject("Z:/");
#else
//    result = c.eject("/dev/sdd");
#endif

    if (result)
        qDebug() << "Ejecting succeeded";
    else
        qDebug() << "Ejecting failed:" << c.errorString();
}
