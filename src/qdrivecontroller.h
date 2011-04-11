#ifndef QDRIVECONTROLLER_H
#define QDRIVECONTROLLER_H

#include <QtCore/QObject>

#include "qdrive_global.h"

class QDriveControllerPrivate;
class QDRIVEINFO_EXPORT QDriveController : public QObject
{
    Q_OBJECT

public:
    explicit QDriveController(QObject *parent = 0);
    ~QDriveController();

    enum MountError { MountErrorNone = 0,
                      MountErrorAccessDenied = 0x01,
                      MountErrorBadDevice = 0x02,
                      MountErrorBadMountPoint = 0x07,
                      MountErrorInvalidCredentials = 0x03,
                      MountErrorResourceBusy = 0x04,
                      MountErrorAlreadyMounted = 0x05,
                      MountErrorNetworkError = 0x06,
                      MountErrorUnknown = 0xff };

    Q_ENUMS(MountError);

    MountError error() const;
    QString errorString() const;

    bool mount(const QString &device, const QString &path);
    bool unmount(const QString &path);
    bool eject(const QString &path);

Q_SIGNALS:
    void driveMounted(const QString &path);
    void driveUnmounted(const QString &path);

protected:
    Q_DECLARE_PRIVATE(QDriveController);
    QDriveControllerPrivate *const d;
};

#endif // QDRIVECONTROLLER_H
