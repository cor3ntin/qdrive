#include "qdriveinfo.h"
#include "qdriveinfo_p.h"

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QStringList>

//#include <sys/param.h>
#include <sys/mount.h>

#include <CoreServices/CoreServices.h>
//#include <IOKit/storage/IOCDBlockStorageDriver.h>
#include <IOKit/storage/IOCDMedia.h>
#include <IOKit/storage/IODVDMedia.h>

QStringList drivePaths()
{
    qDebug("drivePaths");
    QStringList paths;

    OSErr result = noErr;
    ItemCount volumeIndex;

    for (volumeIndex = 1; result == noErr || result != nsvErr; volumeIndex++) {

        FSVolumeRefNum actualVolume;
        FSRef rootDirectory;

        result = FSGetVolumeInfo(kFSInvalidVolumeRefNum,
                                 volumeIndex,
                                 &actualVolume,
                                 0,
                                 0,
                                 0,
                                 &rootDirectory);

        if (result == noErr) {
            CFURLRef url = CFURLCreateFromFSRef(NULL, &rootDirectory);
            CFStringRef stringRef = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
            if (stringRef)	{
                CFIndex length = CFStringGetLength(stringRef) + 1;
                char *volname = NewPtr(length);
                CFStringGetCString(stringRef,
                                   volname,
                                   length,
                                   kCFStringEncodingMacRoman
                                   );
                CFRelease(stringRef);
                paths.append(QString(volname));
                DisposePtr(volname);
            }
            CFRelease(url);
        }
    }
    return paths;
}

QList<QDriveInfo> QDriveInfoPrivate::drives()
{
    QList<QDriveInfo> result;
    foreach (QString path, drivePaths()) {
        result.append(QDriveInfo(path));
    }
    return result;
}

void QDriveInfoPrivate::stat(uint requiredFlags)
{
    if (data->getCachedFlag(requiredFlags))
        return;

    data.detach();

    uint bitmask = 0;
    if (requiredFlags & CachedReadyFlag) {
        data->ready = true;
        data->setCachedFlag(CachedReadyFlag);
    }

    bitmask = CachedAvailableSizeFlag | CachedFreeSizeFlag | CachedSizeFlag |
              CachedFileSystemNameFlag | CachedDeviceFlag;
    if (requiredFlags & bitmask) {
        statFS();
        data->setCachedFlag(bitmask);
    }

    bitmask = CachedNameFlag;
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
    struct statfs statFS;

    statfs(data->rootPath.toUtf8().data(), &statFS);

    data->totalSize = statFS.f_blocks*statFS.f_bsize;
    data->freeSize = statFS.f_bfree*statFS.f_bsize;
    data->availableSize = statFS.f_bavail*statFS.f_bsize;

//    setCachedFlag(CachedAvailableSizeFlag |
//                  CachedFreeSizeFlag |
//                  CachedSizeFlag);
    data->fileSystemName = QString::fromUtf8(statFS.f_fstypename);
    data->device = QString::fromUtf8(statFS.f_mntfromname);
}

FSVolumeRefNum getVolumeRefNumForPath(char *path)
{
    FSRef ref;
    FSPathMakeRef((UInt8*)path, &ref, 0);
    FSCatalogInfo catalogInfo;
    OSErr result = FSGetCatalogInfo(&ref,
                                    kFSCatInfoVolume,
                                    &catalogInfo,
                                    NULL,
                                    NULL,
                                    NULL
                                    );
    if (noErr == result) {
        return catalogInfo.volume;
    }
    return kFSInvalidVolumeRefNum;
}

void QDriveInfoPrivate::getVolumeInfo()
{
    OSErr result = noErr;
    FSVolumeRefNum thisVolumeRefNum = getVolumeRefNumForPath(data->rootPath.toUtf8().data());
    HFSUniStr255 volumeName;
//    FSVolumeInfo volumeInfo;

//    bzero((void *) &volumeInfo, sizeof(volumeInfo));

    result = FSGetVolumeInfo(thisVolumeRefNum,
                             0,
                             0,
                             kFSVolInfoFSInfo,
                             0/*&volumeInfo*/,
                             &volumeName,
                             0);
    if (result == noErr) {
        CFStringRef stringRef = FSCreateStringFromHFSUniStr(NULL, &volumeName);
        if (stringRef) {
            char *volname = NewPtr(CFStringGetLength(stringRef)+1);
            CFStringGetCString(stringRef,
                               volname,
                               CFStringGetLength(stringRef) + 1,
                               kCFStringEncodingMacRoman
                               );
            data->name = QString(volname);
            CFRelease(stringRef);
            DisposePtr(volname);
        }

    }
}

void QDriveInfoPrivate::getType()
{
    stat(CachedDeviceFlag); // we need BSD device to determine drive type.
    data->type = determineType(data->device.toLatin1());
}

// From Qt Mobility
QDriveInfo::DriveType QDriveInfoPrivate::determineType(const QByteArray &device)
{
    QDriveInfo::DriveType drivetype = QDriveInfo::NoDrive;

    DADiskRef diskRef;
    DASessionRef sessionRef;
    CFBooleanRef boolRef;
    CFBooleanRef boolRef2;
    CFDictionaryRef descriptionDictionary;

    sessionRef = DASessionCreate(NULL);
    if (sessionRef == NULL) {
        return QDriveInfo::NoDrive;
    }

    diskRef = DADiskCreateFromBSDName(NULL,
                                      sessionRef,
                                      device
                                      /*data->device.toLatin1()*/
                                      /*mountEntriesMap.key(driveVolume).toLatin1()*/);
    if (diskRef == NULL) {
        CFRelease(sessionRef);
        return QDriveInfo::NoDrive;
    }

    descriptionDictionary = DADiskCopyDescription(diskRef);
    if (descriptionDictionary == NULL) {
        CFRelease(diskRef);
        CFRelease(sessionRef);
        return QDriveInfo::RemoteDrive;
    }

    boolRef = (CFBooleanRef)
              CFDictionaryGetValue(descriptionDictionary, kDADiskDescriptionMediaRemovableKey);
    if (boolRef) {
        if(CFBooleanGetValue(boolRef)) {
            drivetype = QDriveInfo::RemovableDrive;
        } else {
            drivetype = QDriveInfo::InternalDrive;
        }
    }
    boolRef2 = (CFBooleanRef)
              CFDictionaryGetValue(descriptionDictionary, kDADiskDescriptionVolumeNetworkKey);
    if (boolRef2) {
        if(CFBooleanGetValue(boolRef2)) {
            drivetype = QDriveInfo::RemoteDrive;
        }
    }

    DADiskRef wholeDisk;
    wholeDisk = DADiskCopyWholeDisk(diskRef);

    if (wholeDisk) {
        io_service_t mediaService;

        mediaService = DADiskCopyIOMedia(wholeDisk);
        if (mediaService) {
            if (IOObjectConformsTo(mediaService, kIOCDMediaClass)) {
                drivetype = QDriveInfo::CdromDrive;
            }
            if (IOObjectConformsTo(mediaService, kIODVDMediaClass)) {
                drivetype = QDriveInfo::CdromDrive;
            }
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
