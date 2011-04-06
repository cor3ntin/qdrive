#include "qdrivecontroller_p.h"

#include <QtCore/QCoreApplication>

Watcher *Watcher::watcher = 0;

Watcher::Watcher(QObject *parent) :
        QObject(parent),
        CActive(EPriorityStandard)
{

}

Watcher::~Watcher()
{
}

void Watcher::DoCancel()
{
}

void Watcher::RunL()
{
}

QDriveControllerPrivate::QDriveControllerPrivate()
{
    if (!Watcher::watcher) {
        // TODO: add mutex
        Watcher::watcher = new Watcher(qApp);
    }
}

QDriveControllerPrivate::~QDriveControllerPrivate()
{

}
