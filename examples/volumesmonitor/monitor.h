#ifndef MONITOR_H
#define MONITOR_H

#include <QtCore/QObject>

class Monitor : public QObject
{
    Q_OBJECT

public:
    explicit Monitor(QObject *parent = 0);
    ~Monitor();

public Q_SLOTS:
    void driveMounted(const QString &path);
    void driveUnmounted(const QString &path);
};

#endif // MONITOR_H
