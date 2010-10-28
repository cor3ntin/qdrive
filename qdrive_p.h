#ifndef QDRIVEINFO_P_H
#define QDRIVEINFO_P_H

#include "qdrive.h"

class QDrivePrivateBase
{
public:
    QDrivePrivateBase() {}

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
        CachedDeviceFlag = 0x100
    };

    inline bool getCachedFlag(uint c) const
    { return cache_enabled ? (cachedFlags & c) : 0; }
    inline void setCachedFlag(uint c)
    { if (cache_enabled) cachedFlags |= c; }

    quint64 availableSize;
    QString fileSystemName;
    quint64 freeSize;
    QString device;
    QString name;
    bool    ready;
    QString rootPath;
    quint64 size;

    bool cache_enabled;
    uint cachedFlags;
};

#endif // QDRIVEINFO_P_H
