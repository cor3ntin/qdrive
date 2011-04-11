#include "monitor.h"

#include <QtCore/QDebug>

#include <QDriveController>

Monitor::Monitor(QObject *parent) : QObject(parent)
{
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
