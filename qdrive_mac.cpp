#include "qdrive_mac_p.h"

//#include <sys/param.h>
#include <sys/mount.h>

#include <CoreServices/CoreServices.h>

//================================== QDriveInfoPrivate ==================================

QDrivePrivate::QDrivePrivate()
{
}

void QDrivePrivate::stat(uint requiredFlags)
{
    uint bitmask = 0;
    if (requiredFlags & CachedReadyFlag) {
        ready = true;
        setCachedFlag(CachedReadyFlag);
    }

    bitmask = CachedAvailableSizeFlag | CachedFreeSizeFlag | CachedSizeFlag |
              CachedFileSystemNameFlag | CachedDeviceFlag;
    if (requiredFlags & bitmask &&
        !getCachedFlag(bitmask))
        statFS();

    bitmask = CachedNameFlag;
    if (requiredFlags & bitmask &&
        !getCachedFlag(bitmask))
        getVolumeInfo();

    bitmask = CachedTypeFlag;
    if (requiredFlags & bitmask &&
        !getCachedFlag(bitmask))
        getType();
}

void QDrivePrivate::statFS()
{
    struct statfs statFS;
    statfs(rootPath.toUtf8().data(), &statFS);

    size = statFS.f_blocks*statFS.f_bsize;
    freeSize = statFS.f_bfree*statFS.f_bsize;
    availableSize = statFS.f_bavail*statFS.f_bsize;

    setCachedFlag(CachedAvailableSizeFlag |
                  CachedFreeSizeFlag |
                  CachedSizeFlag);
    fileSystemName = QString(statFS.f_fstypename);
    device = QString(statFS.f_mntfromname);
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

void QDrivePrivate::getVolumeInfo()
{
    OSErr result = noErr;
    FSVolumeRefNum thisVolumeRefNum = getVolumeRefNumForPath(rootPath.toUtf8().data());
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
            name = QString(volname);
            CFRelease(stringRef);
            DisposePtr(volname);
        }

    }
}

bool QDrivePrivate::setName(const QString &name)
{
    return false;
}

void QDrivePrivate::getType()
{
    type = QDrive::NoDrive;

    setCachedFlag(CachedTypeFlag);
}

QStringList QDrive::drivePaths()
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
