#include <QtCore/QCoreApplication>

#include <QTimer>

#include <QDriveController>

#include "monitor.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QDriveController c(&app);

    Monitor m(&c);

    QTimer::singleShot(15000, &app, SLOT(quit()));

    return app.exec();
}
