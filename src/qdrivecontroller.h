#ifndef QDRIVECONTROLLER_H
#define QDRIVECONTROLLER_H

#include <QtCore/QObject>

#include "qdrive_global.h"

class QDRIVEINFO_EXPORT QDriveController : public QObject
{
    Q_OBJECT

public:
    explicit QDriveController(QObject *parent = 0);
    ~QDriveController();

    bool mount(const QString &device, const QString &path);
    bool unmount(const QString &path);
    bool eject(const QString &path);

Q_SIGNALS:
    void driveMounted(const QString &path);
    void driveUnmounted(const QString &path);
// ### removeme!
public Q_SLOTS:
    void testDriveMounted(const QString &path);
    void testDriveUnounted(const QString &path);
//
};

#endif // QDRIVECONTROLLER_H
