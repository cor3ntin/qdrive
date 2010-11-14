#ifndef QDRIVEINFO_H
#define QDRIVEINFO_H

#include "qdriveinfo_global.h"

#include <QtCore/QList>
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

    enum Capability {
        CaseSensitiveFileNames = 0x00000001,
        AccessControlListsSupport = 0x00000002,
        HardlinksSupport = 0x00000004,
        SymlinksSupport = 0x00000008,
        ReadOnlyVolume = 0x00000010
    };
    Q_DECLARE_FLAGS(Capabilities, Capability)

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
    bool isReady() const;
    bool isValid() const;

    DriveType type() const;

    Capabilities capabilities() const;

    void refresh();

    static QList<QDriveInfo> drives();

protected:
    QDriveInfoPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(QDriveInfo);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QDriveInfo::Capabilities)

#endif // QDRIVEINFO_H
