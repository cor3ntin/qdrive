#include "qdriveinfo_p.h"

//#include <sys/param.h>
#include <sys/mount.h>

#include <CoreServices/CoreServices.h>
//#include <IOKit/storage/IOCDBlockStorageDriver.h>
#include <IOKit/storage/IOCDMedia.h>
#include <IOKit/storage/IODVDMedia.h>

void QDriveInfoPrivate::initRootPath()
{
    if (data->rootPath.isEmpty())
        return;

    FSRef ref;
    FSPathMakeRef((UInt8*)QFile::encodeName(data->rootPath).constData(), &ref, 0);

    FSCatalogInfo catalogInfo;
    if (FSGetCatalogInfo(&ref, kFSCatInfoVolume, &catalogInfo, 0, 0, 0) != noErr)
        return;

    HFSUniStr255 volumeName;
    FSRef rootDirectory;
    if (FSGetVolumeInfo(catalogInfo.volume, 0, 0, kFSVolInfoFSInfo, 0, &volumeName, &rootDirectory) == noErr) {
        CFStringRef stringRef = FSCreateStringFromHFSUniStr(NULL, &volumeName);
        if (stringRef) {
            CFIndex length = CFStringGetLength(stringRef) + 1;
            char *volname = NewPtr(length);
            CFStringGetCString(stringRef, volname, length, kCFStringEncodingMacRoman);
            data->name = QFile::decodeName(volname);
            CFRelease(stringRef);
            DisposePtr(volname);
        }

        CFURLRef url = CFURLCreateFromFSRef(NULL, &rootDirectory);
        stringRef = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
        if (stringRef) {
            CFIndex length = CFStringGetLength(stringRef) + 1;
            char *volname = NewPtr(length);
            CFStringGetCString(stringRef, volname, length, kCFStringEncodingMacRoman);
            data->rootPath = QFile::decodeName(volname);
            CFRelease(stringRef);
            DisposePtr(volname);
        }
        CFRelease(url);
    }
}

static inline QDriveInfo::DriveType determineType(const QString &device)
{
    QDriveInfo::DriveType drivetype = QDriveInfo::InvalidDrive;

    DADiskRef diskRef;
    DASessionRef sessionRef;
    CFBooleanRef boolRef;
    CFBooleanRef boolRef2;
    CFDictionaryRef descriptionDictionary;

    sessionRef = DASessionCreate(NULL);
    if (sessionRef == NULL)
        return QDriveInfo::InvalidDrive;

    diskRef = DADiskCreateFromBSDName(NULL, sessionRef, QFile::encodeName(device).constData());
    if (diskRef == NULL) {
        CFRelease(sessionRef);
        return QDriveInfo::InvalidDrive;
    }

    descriptionDictionary = DADiskCopyDescription(diskRef);
    if (descriptionDictionary == NULL) {
        CFRelease(diskRef);
        CFRelease(sessionRef);
        return QDriveInfo::RemoteDrive;
    }

    boolRef = (CFBooleanRef)CFDictionaryGetValue(descriptionDictionary, kDADiskDescriptionMediaRemovableKey);
    if (boolRef)
        drivetype = CFBooleanGetValue(boolRef) ? QDriveInfo::RemovableDrive : QDriveInfo::InternalDrive;
    boolRef2 = (CFBooleanRef)CFDictionaryGetValue(descriptionDictionary, kDADiskDescriptionVolumeNetworkKey);
    if (boolRef2 && CFBooleanGetValue(boolRef2))
        drivetype = QDriveInfo::RemoteDrive;

    DADiskRef wholeDisk;
    wholeDisk = DADiskCopyWholeDisk(diskRef);
    if (wholeDisk) {
        io_service_t mediaService;
        mediaService = DADiskCopyIOMedia(wholeDisk);
        if (mediaService) {
            if (IOObjectConformsTo(mediaService, kIOCDMediaClass))
                drivetype = QDriveInfo::CdromDrive;
            else if (IOObjectConformsTo(mediaService, kIODVDMediaClass))
                drivetype = QDriveInfo::CdromDrive;
            IOObjectRelease(mediaService);
        }
        CFRelease(wholeDisk);
    }
    CFRelease(diskRef);
    CFRelease(descriptionDictionary);
    CFRelease(boolRef);
    CFRelease(boolRef2);
    CFRelease(sessionRef);

    return drivetype;
}

void QDriveInfoPrivate::doStat(uint requiredFlags)
{
    if (data->getCachedFlag(requiredFlags))
        return;

    if (!data->getCachedFlag(CachedRootPathFlag)) {
        initRootPath();
        data->setCachedFlag(CachedRootPathFlag | CachedNameFlag);
    }

    if (data->rootPath.isEmpty() || (data->getCachedFlag(CachedValidFlag) && !data->valid))
        return;

    if (!data->getCachedFlag(CachedValidFlag))
        requiredFlags |= CachedValidFlag; // force drive validation

    if (requiredFlags & CachedTypeFlag)
        requiredFlags |= CachedDeviceFlag;


    uint bitmask = 0;

    bitmask = CachedAvailableSizeFlag | CachedFreeSizeFlag | CachedSizeFlag | CachedFileSystemNameFlag |
              CachedDeviceFlag | CachedValidFlag | CachedReadyFlag | CachedCapabilitiesFlag;
    if (requiredFlags & bitmask) {
        statFS();
        data->setCachedFlag(bitmask);

        if (!data->valid)
            return;
    }

    bitmask = CachedTypeFlag;
    if (requiredFlags & bitmask) {
        data->type = determineType(data->device);
        data->setCachedFlag(bitmask);
    }
}

void QDriveInfoPrivate::statFS()
{
    struct statfs statfs_buf;
    int result = ::statfs(QFile::encodeName(data->rootPath).constData(), &statfs_buf);
    if (result == -1) {
        data->valid = false;
        data->ready = false;
    } else {
        data->valid = true;
        data->ready = true;

        data->totalSize = statfs_buf.f_blocks * statfs_buf.f_bsize;
        data->freeSize = statfs_buf.f_bfree * statfs_buf.f_bsize;
        data->availableSize = statfs_buf.f_bavail * statfs_buf.f_bsize;

        data->fileSystemName = QString::fromLatin1(statfs_buf.f_fstypename);
        data->device = QFile::decodeName(statfs_buf.f_mntfromname);

        // try to determine flags
        if (statfs_buf.f_flags & MNT_RDONLY)
            data->capabilities |= QDriveInfo::ReadOnlyVolume;

        FSRef ref;
        FSPathMakeRef((UInt8*)QFile::encodeName(data->rootPath).constData(), &ref, 0);

        FSCatalogInfo catalogInfo;
        if (FSGetCatalogInfo(&ref, kFSCatInfoVolume, &catalogInfo, 0, 0, 0) != noErr)
            return;

        GetVolParmsInfoBuffer infoBuffer;
        FSGetVolumeParms(catalogInfo.volume, &infoBuffer, sizeof(infoBuffer));
        // foo, hardcoded. TODO: add all OTHER filesystems. Good luck.
        if (data->fileSystemName == "hfs")
            data->capabilities |= QDriveInfo::HardlinksSupport;
        if (infoBuffer.vMExtendedAttributes & bSupportsSymbolicLinks)
            data->capabilities |= QDriveInfo::SymlinksSupport;
        if (infoBuffer.vMExtendedAttributes & bIsCaseSensitive)
            data->capabilities |= QDriveInfo::CaseSensitiveFileNames;
    }
}

QList<QDriveInfo> QDriveInfoPrivate::drives()
{
    QList<QDriveInfo> drives;

    OSErr result = noErr;
    for (ItemCount volumeIndex = 1; result == noErr || result != nsvErr; volumeIndex++) {
        FSVolumeRefNum actualVolume;
        FSRef rootDirectory;
        result = FSGetVolumeInfo(kFSInvalidVolumeRefNum, volumeIndex, &actualVolume, 0, 0, 0, &rootDirectory);
        if (result == noErr) {
            CFURLRef url = CFURLCreateFromFSRef(NULL, &rootDirectory);
            CFStringRef stringRef = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
            if (stringRef) {
                CFIndex length = CFStringGetLength(stringRef) + 1;
                char *volname = NewPtr(length);
                CFStringGetCString(stringRef, volname, length, kCFStringEncodingMacRoman);
                drives.append(QDriveInfo(QFile::decodeName(volname)));
                CFRelease(stringRef);
                DisposePtr(volname);
            }
            CFRelease(url);
        }
    }

    return drives;
}
