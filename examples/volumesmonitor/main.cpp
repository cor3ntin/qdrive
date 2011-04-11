#include <QtCore/QCoreApplication>

#include <QTimer>

#include <QDriveController>

#include "monitor.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QDriveController c;

    Monitor m;

    app.connect(&c, SIGNAL(driveMounted(QString)), &m, SLOT(driveMounted(QString)));
    app.connect(&c, SIGNAL(driveUnmounted(QString)), &m, SLOT(driveUnmounted(QString)));

    QTimer::singleShot(15000, &app, SLOT(quit()));

    return app.exec();
}
