#ifndef QDRIVECONTROLLER_H
#define QDRIVECONTROLLER_H

#include <QtCore/QObject>

#include "qtdriveinfoglobal.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QDriveControllerPrivate;
class Q_DRIVEINFO_EXPORT QDriveController : public QObject
{
    Q_OBJECT

public:
    explicit QDriveController(QObject *parent = 0);
    ~QDriveController();

    int error() const;
    QString errorString() const;

    bool mount(const QString &device, const QString &path = QString());
    bool unmount(const QString &path);
    bool eject(const QString &path);

Q_SIGNALS:
    void driveMounted(const QString &path);
    void driveUnmounted(const QString &path);

protected:
    QDriveControllerPrivate *const d;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QDRIVECONTROLLER_H
