#ifndef QDRIVEINFO_H
#define QDRIVEINFO_H

#include "qdriveinfo_global.h"

#include <QtCore/QString>

class QDriveInfoPrivate;
class QDRIVEINFO_EXPORT QDriveInfo
{
public:
    enum DriveType {
        InvalidDrive = 0,
        InternalDrive,
        RemovableDrive,
        RemoteDrive,
        CdromDrive,
        InternalFlashDrive,
        RamDrive
    };

    QDriveInfo();
    explicit QDriveInfo(const QString &rootPath);
    QDriveInfo(const QDriveInfo &other);
    ~QDriveInfo();

    QDriveInfo &operator=(const QDriveInfo &other);

    QString rootPath() const;
    void setRootPath(const QString &);

    quint64 availableSize() const;
    quint64 freeSize() const;
    quint64 totalSize() const;

    QString fileSystemName() const;
    QString device() const;
    QString name() const;
    bool ready() const;
    bool isValid() const;

    DriveType type() const;

    void refresh();

    static QList<QDriveInfo> drives();

protected:
    QDriveInfoPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(QDriveInfo);
};

#endif // QDRIVEINFO_H
