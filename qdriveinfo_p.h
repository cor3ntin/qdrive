#ifndef QDRIVEINFO_P_H
#define QDRIVEINFO_P_H

#include "qdriveinfo.h"

#include <QtCore/QExplicitlySharedDataPointer>
#include <QtCore/QFile>

#include <qplatformdefs.h>

class QDriveInfoPrivate
{
public:
    QDriveInfoPrivate();
    QDriveInfoPrivate(QDriveInfoPrivate *other);

    enum CachedFlags {
        CachedRootPathFlag = 0x001,
        CachedDeviceFlag = 0x002,
        CachedFileSystemNameFlag = 0x004,
        CachedNameFlag = 0x008,
        CachedTotalSizeFlag = 0x010,
        CachedFreeSizeFlag = 0x020,
        CachedAvailableSizeFlag = 0x040,
        CachedTypeFlag = 0x100,
        CachedCapabilitiesFlag = 0x200,
        CachedReadyFlag = 0x400,
        CachedValidFlag = 0x800
    };

    struct Data : public QSharedData
    {
        Data() : QSharedData(),
            totalSize(0), freeSize(0), availableSize(0),
            type(QDriveInfo::InvalidDrive), capabilities(0),
            ready(false), valid(false),
            cachedFlags(0)
        {}
        Data(const Data &other) : QSharedData(other),
            rootPath(other.rootPath),
            totalSize(0), freeSize(0), availableSize(0),
            type(QDriveInfo::InvalidDrive), capabilities(0),
            ready(false), valid(false),
            cachedFlags(0)
        {}

        inline void clear()
        {
            device.clear();
            fileSystemName.clear();
            name.clear();

            totalSize = 0;
            freeSize = 0;
            availableSize = 0;

            type = QDriveInfo::InvalidDrive;
            capabilities = 0;
            ready = false;
            valid = false;

            cachedFlags = 0;
        }

        inline bool getCachedFlag(uint c) const
        { return (cachedFlags & c); }
        inline void setCachedFlag(uint c)
        { cachedFlags |= c; }

        QString rootPath;
        QString device;
        QString fileSystemName;
        QString name;

        quint64 totalSize;
        quint64 freeSize;
        quint64 availableSize;

        QDriveInfo::DriveType type;
        uint capabilities;
        bool ready;
        bool valid;

        uint cachedFlags;
    };
    QExplicitlySharedDataPointer<Data> data;

    void initRootPath();
    void doStat(uint requiredFlags);

    static QList<QDriveInfo> drives();

protected:
    void getVolumeInfo();
#if defined(Q_OS_WIN)
    void getDiskFreeSpace();
#endif
};

#endif // QDRIVEINFO_P_H
