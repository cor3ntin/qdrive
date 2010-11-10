#include "qdriveinfo.h"
#include "qdriveinfo_p.h"

//#include <sys/param.h>
#include <sys/mount.h>

#include <CoreServices/CoreServices.h>
//#include <IOKit/storage/IOCDBlockStorageDriver.h>
#include <IOKit/storage/IOCDMedia.h>
#include <IOKit/storage/IODVDMedia.h>

QList<QDriveInfo> QDriveInfoPrivate::drives()
{
    QList<QDriveInfo> drives;

    OSErr result = noErr;
    ItemCount volumeIndex;

    for (volumeIndex = 1; result == noErr || result != nsvErr; volumeIndex++) {
        FSVolumeRefNum actualVolume;
        FSRef rootDirectory;

        result = FSGetVolumeInfo(kFSInvalidVolumeRefNum,
                                 volumeIndex,
                                 &actualVolume,
                                 0, 0, 0,
                                 &rootDirectory);
        if (result == noErr) {
            CFURLRef url = CFURLCreateFromFSRef(NULL, &rootDirectory);
            CFStringRef stringRef = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
            if (stringRef) {
                CFIndex length = CFStringGetLength(stringRef) + 1;
                char *volname = NewPtr(length);
                CFStringGetCString(stringRef, volname, length, kCFStringEncodingMacRoman);
                drives.append(QString::fromLocal8Bit(volname));
                CFRelease(stringRef);
                DisposePtr(volname);
            }
            CFRelease(url);
        }
    }

    return drives;
}

void QDriveInfoPrivate::doStat(uint requiredFlags)
{
    if (data->getCachedFlag(requiredFlags))
        return;

    uint bitmask = 0;

    bitmask = CachedAvailableSizeFlag | CachedFreeSizeFlag | CachedSizeFlag |
              CachedFileSystemNameFlag | CachedDeviceFlag | CachedValidFlag | CachedReadyFlag;
    if (requiredFlags & bitmask) {
        statFS();
        data->setCachedFlag(bitmask);
    }

    bitmask = CachedNameFlag | CachedRootPathFlag;
    if (requiredFlags & bitmask) {
        getVolumeInfo();
        data->setCachedFlag(bitmask);
    }

    bitmask = CachedTypeFlag;
    if (requiredFlags & bitmask) {
        getType();
        data->setCachedFlag(bitmask);
    }
}

void QDriveInfoPrivate::statFS()
{
    struct statfs statfs_buf;
    int result = statfs(data->rootPath.toUtf8().data(), &statfs_buf);
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
        data->device = QString::fromLocal8Bit(statfs_buf.f_mntfromname);
    }
}

static inline FSVolumeRefNum getVolumeRefNumForPath(char *path)
{
    FSRef ref;
    FSPathMakeRef((UInt8*)path, &ref, 0);
    FSCatalogInfo catalogInfo;
    OSErr result = FSGetCatalogInfo(&ref,
                                    kFSCatInfoVolume,
                                    &catalogInfo,
                                    NULL,
                                    NULL,
                                    NULL);
    if (noErr == result)
        return catalogInfo.volume;

    return kFSInvalidVolumeRefNum;
}

void QDriveInfoPrivate::getVolumeInfo()
{
    OSErr result = noErr;
    FSVolumeRefNum thisVolumeRefNum = getVolumeRefNumForPath(data->rootPath.toUtf8().data());
    HFSUniStr255 volumeName;
    FSRef rootDirectory;

    result = FSGetVolumeInfo(thisVolumeRefNum,
                             0,
                             0,
                             kFSVolInfoFSInfo,
                             0,
                             &volumeName,
                             &rootDirectory);
    if (result == noErr) {
        CFStringRef stringRef = FSCreateStringFromHFSUniStr(NULL, &volumeName);
        if (stringRef) {
            CFIndex length = CFStringGetLength(stringRef) + 1;
            char *volname = NewPtr(length);
            CFStringGetCString(stringRef, volname, length, kCFStringEncodingMacRoman);
            data->name = QString::fromLocal8Bit(volname);
            CFRelease(stringRef);
            DisposePtr(volname);
        }

        CFURLRef url = CFURLCreateFromFSRef(NULL, &rootDirectory);
        stringRef = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
        if (stringRef) {
            CFIndex length = CFStringGetLength(stringRef) + 1;
            char *volname = NewPtr(length);
            CFStringGetCString(stringRef, volname, length, kCFStringEncodingMacRoman);
            data->rootPath = QString::fromLocal8Bit(volname);
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

    diskRef = DADiskCreateFromBSDName(NULL, sessionRef, device.toLocal8Bit());
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

void QDriveInfoPrivate::getType()
{
    doStat(CachedDeviceFlag); // we need BSD device to determine drive type.

    data->type = determineType(data->device);
}
