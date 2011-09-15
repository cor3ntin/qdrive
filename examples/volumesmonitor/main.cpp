#include <QtCore/QCoreApplication>

#include <QTimer>

#include <QDriveController>

#include "monitor.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QDriveController c(&app);

    Monitor m(&c);

    QTimer::singleShot(1000, &m, SLOT(testMount()));
//    QTimer::singleShot(1500, &m, SLOT(testUnmount()));
    QTimer::singleShot(2000, &m, SLOT(testEject()));

    QTimer::singleShot(15000, &app, SLOT(quit()));

    return app.exec();
}
