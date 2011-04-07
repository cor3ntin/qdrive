#ifndef QDRIVECONTROLLER_H
#define QDRIVECONTROLLER_H

#include <QObject>

#include "qdrive_global.h"

class QDriveControllerPrivate;
class QDRIVEINFO_EXPORT QDriveController : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QDriveController)
public:
    explicit QDriveController(QObject *parent = 0);

signals:
    void driveMounted(const QString &mountPath);
    void driveUnmounted(const QString &mountPath);

public slots:

protected:
    QDriveControllerPrivate *d_ptr;

};

#endif // QDRIVECONTROLLER_H
