#include "qdrivecontroller.h"
#include "qdrivecontroller_p.h"

QDriveController::QDriveController(QObject *parent) :
    QObject(parent),
    d_ptr(new QDriveControllerPrivate)
{
}


