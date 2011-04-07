#ifndef QDRIVECONTROLLER_H
#define QDRIVECONTROLLER_H

#include <QtCore/QObject>

#include "qdrive_global.h"

class QDriveControllerPrivate;
class QDRIVEINFO_EXPORT QDriveController : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QDriveController)
public:
    explicit QDriveController(QObject *parent = 0);
    ~QDriveController();

signals:
    void driveMounted(const QString &path);
    void driveUnmounted(const QString &path);

public slots:
    void testDriveMounted(const QString &path);
    void testDriveUnounted(const QString &path);

protected:
    QDriveControllerPrivate *d_ptr;
};

#endif // QDRIVECONTROLLER_H
