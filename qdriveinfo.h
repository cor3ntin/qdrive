#ifndef QDRIVEINFO_H
#define QDRIVEINFO_H

#include <QtCore/QString>

class QDriveInfoPrivate;
class QDriveInfo
{
    Q_DECLARE_PRIVATE(QDriveInfo);

public:
    QDriveInfo();
    explicit QDriveInfo(const QString &rootPath);
    QDriveInfo(const QDriveInfo &);
    QDriveInfo &operator=(const QDriveInfo &);
    virtual ~QDriveInfo();

    enum DriveType {
        NoDrive = 0,
        InternalDrive,
        RemovableDrive,
        RemoteDrive,
        CdromDrive,
        InternalFlashDrive,
        RamDrive
    };

    static QList<QDriveInfo> drives();

    quint64 availableSize() const;
    quint64 freeSize() const;
    quint64 totalSize() const;

    QString fileSystemName() const;
    QString device() const;
    QString name() const;
    bool    ready() const;
    bool    isValid() const;

    QString rootPath() const;
    void setRootPath(const QString &);

    DriveType type() const;

protected:
    QDriveInfoPrivate *d_ptr;
};

#endif // QDRIVEINFO_H
