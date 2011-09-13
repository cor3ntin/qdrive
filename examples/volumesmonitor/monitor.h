#ifndef MONITOR_H
#define MONITOR_H

#include <QtCore/QObject>

class QDriveController;

class Monitor : public QObject
{
    Q_OBJECT

public:
    Monitor(QDriveController *controller);
    ~Monitor();

public Q_SLOTS:
    void driveMounted(const QString &path);
    void driveUnmounted(const QString &path);

    void testMount();
    void testUnmount();
};

#endif // MONITOR_H
