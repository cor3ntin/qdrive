#ifndef QDRIVEINFO_P_H
#define QDRIVEINFO_P_H

#include "qdriveinfo.h"
#include <QtCore/QExplicitlySharedDataPointer>
#include <QtCore/QStringList>

class QDriveInfoPrivate
{
public:
    QDriveInfoPrivate();
    QDriveInfoPrivate(QDriveInfoPrivate *other);

    enum CachedFlags {
        CachedNoFlags = 0x00,
        CachedAvailableSizeFlag = 0x01,
        CachedFreeSizeFlag = 0x02,
        CachedSizeFlag = 0x04,
        CachedTotalFreeSizeFlag = 0x08,
        CachedFileSystemNameFlag = 0x10,
        CachedNameFlag = 0x20,
        CachedRootPathFlag = 0x40,
        CachedReadyFlag = 0x80,
        CachedDeviceFlag = 0x100,
        CachedTypeFlag = 0x200,
        CachedValidFlag = 0x400
    };

    struct Data : public QSharedData
    {
        Data() : QSharedData(),
            availableSize(0), freeSize(0), totalSize(0),
            type(QDriveInfo::InvalidDrive), ready(false), valid(false),
            cache_enabled(true), cachedFlags(0)
        {}
        Data(const Data &other) : QSharedData(other),
            rootPath(other.rootPath),
            availableSize(0), freeSize(0), totalSize(0),
            type(QDriveInfo::InvalidDrive), ready(false), valid(false),
            cache_enabled(other.cache_enabled), cachedFlags(0)
        {}

        QString rootPath;

        quint64 availableSize;
        quint64 freeSize;
        quint64 totalSize;

        QString fileSystemName;
        QString device;
        QString name;
        QDriveInfo::DriveType type;
        bool ready;
        bool valid;

        bool cache_enabled;
        uint cachedFlags;

        inline bool getCachedFlag(uint c) const
        { return cache_enabled ? (cachedFlags & c) : 0; }
        inline void setCachedFlag(uint c)
        { if (cache_enabled) cachedFlags |= c; }
    };

    static QList<QDriveInfo> drives();

    void stat(uint requiredFlags);

    QExplicitlySharedDataPointer<Data> data;

#ifdef Q_OS_LINUX
#if defined(Q_WS_MAEMO_5) || defined(Q_WS_MAEMO_6)

#else
    void statFS();
    void getMountEntry();
    void getType();
    void getName();
    QDriveInfo::DriveType determineType();
#endif //Q_WS_MAEMO_5 & Q_WS_MAEMO_6
#endif //Q_OS_LINUX

#ifdef Q_OS_WIN
    static QStringList drivePaths();
    void getVolumeInformation();
    void getDiskFreeSpace();
    void getDevice();
    void getType();
    QDriveInfo::DriveType determineType();
#endif
#ifdef Q_OS_MAC
    void statFS();
    void getVolumeInfo();
    void getType();
    QDriveInfo::DriveType determineType(const QByteArray &device);
#endif
#ifdef Q_OS_SYMBIAN

#endif
};

#endif // QDRIVEINFO_P_H
