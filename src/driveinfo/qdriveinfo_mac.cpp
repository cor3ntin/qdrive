/****************************************************************************
**
** Copyright (C) 2012 Ivan Komissarov
** Contact: http://www.qt-project.org/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qdriveinfo_p.h"

#include <CoreServices/CoreServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFURLEnumerator.h>
#include <IOKit/storage/IOCDMedia.h>
#include <IOKit/storage/IODVDMedia.h>

#include <sys/mount.h>
#include <private/qcore_mac_p.h>
#include <QDebug>

#if defined(QT_LARGEFILE_SUPPORT)
#  define QT_STATFSBUF struct statfs64
#  define QT_STATFS    ::statfs64
#else
#  define QT_STATFSBUF struct statfs
#  define QT_STATFS    ::statfs
#endif

QT_BEGIN_NAMESPACE

void QDriveInfoPrivate::initRootPath()
{
    if (rootPath.isEmpty())
        return;

    // deprecated:
    FSRef ref;
    FSPathMakeRef((UInt8*)QFile::encodeName(rootPath).constData(), &ref, 0);

    rootPath.clear();

    // deprecated (use CFURLCopyResourcePropertiesForKeys)
    FSCatalogInfo catalogInfo;
    if (FSGetCatalogInfo(&ref, kFSCatInfoVolume, &catalogInfo, 0, 0, 0) != noErr)
        return;

    // deprecated (use CFURLCopyResourcePropertiesForKeys)
    HFSUniStr255 volumeName;
    FSRef rootDirectory;
    OSErr error = FSGetVolumeInfo(catalogInfo.volume,
                                  0,
                                  0,
                                  kFSVolInfoFSInfo,
                                  0,
                                  &volumeName,
                                  &rootDirectory);
    if (error == noErr) {
        CFURLRef url = CFURLCreateFromFSRef(NULL, &rootDirectory);
        CFStringRef stringRef;
        stringRef = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
        if (stringRef) {
            // TODO : use utf-16 ??
            CFIndex length = CFStringGetLength(stringRef) + 1;
            char *volname = NewPtr(length);
            CFStringGetCString(stringRef, volname, length, kCFStringEncodingMacRoman);
            rootPath = QFile::decodeName(volname);
            CFRelease(stringRef);
            DisposePtr(volname);
        }
        CFRelease(url);

        stringRef = FSCreateStringFromHFSUniStr(NULL, &volumeName);
        if (stringRef) {
            // TODO : use utf-16 ??
            CFIndex length = CFStringGetLength(stringRef) + 1;
            char *volname = NewPtr(length);
            CFStringGetCString(stringRef, volname, length, kCFStringEncodingMacRoman);
            name = QFile::decodeName(volname);
            CFRelease(stringRef);
            DisposePtr(volname);
        }
    }
}

static inline QDriveInfo::DriveType determineType(const QByteArray &device)
{
    QDriveInfo::DriveType drivetype = QDriveInfo::UnknownDrive;

    DASessionRef sessionRef;
    DADiskRef diskRef;
    CFDictionaryRef descriptionDictionary;

    sessionRef = DASessionCreate(NULL);
    if (sessionRef == NULL)
        return QDriveInfo::UnknownDrive;

    diskRef = DADiskCreateFromBSDName(NULL, sessionRef, device.constData());
    if (diskRef == NULL) {
        CFRelease(sessionRef);
        return QDriveInfo::UnknownDrive;
    }

    descriptionDictionary = DADiskCopyDescription(diskRef);
    if (descriptionDictionary == NULL) {
        CFRelease(diskRef);
        CFRelease(sessionRef);
        return QDriveInfo::RemoteDrive;
    }

    CFBooleanRef boolRef;
    boolRef = (CFBooleanRef)CFDictionaryGetValue(descriptionDictionary,
                                                 kDADiskDescriptionVolumeNetworkKey);
    if (boolRef && CFBooleanGetValue(boolRef)){
        CFRelease(descriptionDictionary);
        CFRelease(diskRef);
        CFRelease(sessionRef);
        return QDriveInfo::RemoteDrive;
    }

    boolRef = (CFBooleanRef)CFDictionaryGetValue(descriptionDictionary,
                                                 kDADiskDescriptionMediaRemovableKey);
    if (boolRef)
        drivetype = CFBooleanGetValue(boolRef) ? QDriveInfo::RemovableDrive : QDriveInfo::InternalDrive;

    DADiskRef wholeDisk;
    wholeDisk = DADiskCopyWholeDisk(diskRef);
    if (wholeDisk) {
        io_service_t mediaService;
        mediaService = DADiskCopyIOMedia(wholeDisk);
        if (mediaService) {
            if (IOObjectConformsTo(mediaService, kIOCDMediaClass)
                || IOObjectConformsTo(mediaService, kIODVDMediaClass)) {
                drivetype = QDriveInfo::CdromDrive;
            }
            IOObjectRelease(mediaService);
        }
        CFRelease(wholeDisk);
    }

    CFRelease(descriptionDictionary);
    CFRelease(diskRef);
    CFRelease(sessionRef);

    return drivetype;
}

void QDriveInfoPrivate::doStat(uint requiredFlags)
{
    if (getCachedFlag(requiredFlags))
        return;

    if (!getCachedFlag(CachedRootPathFlag | CachedNameFlag)) {
        initRootPath();
        setCachedFlag(CachedRootPathFlag | CachedNameFlag);
    }

    if (rootPath.isEmpty() || (getCachedFlag(CachedValidFlag) && !valid))
        return;

    if (!getCachedFlag(CachedValidFlag))
        requiredFlags |= CachedValidFlag; // force drive validation

    if (requiredFlags & CachedTypeFlag)
        requiredFlags |= CachedDeviceFlag;


    uint bitmask = 0;

    bitmask = CachedDeviceFlag | CachedFileSystemNameFlag |
              CachedBytesTotalFlag | CachedBytesFreeFlag | CachedBytesAvailableFlag |
              CachedReadOnlyFlag | CachedReadyFlag | CachedValidFlag;
    if (requiredFlags & bitmask) {
        getVolumeInfo();
        setCachedFlag(bitmask);

        if (!valid)
            return;
    }

    bitmask = CachedTypeFlag;
    if (requiredFlags & bitmask) {
        type = determineType(device);
        setCachedFlag(bitmask);
    }
}

void QDriveInfoPrivate::getVolumeInfo()
{
    QT_STATFSBUF statfs_buf;
    // deprecated
    int result = QT_STATFS(QFile::encodeName(rootPath).constData(), &statfs_buf);
    if (result == 0) {
        valid = true;
        ready = true;

        device = QByteArray(statfs_buf.f_mntfromname);
        fileSystemName = QByteArray(statfs_buf.f_fstypename);

        bytesTotal = statfs_buf.f_blocks * statfs_buf.f_bsize;
        bytesFree = statfs_buf.f_bfree * statfs_buf.f_bsize;
        bytesAvailable = statfs_buf.f_bavail * statfs_buf.f_bsize;

        readOnly = (statfs_buf.f_flags & MNT_RDONLY) != 0;
    }
}

QList<QDriveInfo> QDriveInfoPrivate::drives()
{
    QList<QDriveInfo> drives;

    CFURLEnumeratorRef enumerator;
    enumerator = CFURLEnumeratorCreateForMountedVolumes(NULL,
                                                        kCFURLEnumeratorSkipInvisibles,
                                                        NULL);

    CFURLEnumeratorResult result = kCFURLEnumeratorSuccess;
    do {
        CFURLRef url;
        CFErrorRef error;
        result = CFURLEnumeratorGetNextURL(enumerator, &url, &error);
        if (result == kCFURLEnumeratorSuccess) {
            QCFString urlString = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);

            QDriveInfo drive;
            drive.d_ptr->rootPath = urlString;
            drive.d_ptr->setCachedFlag(CachedRootPathFlag);
            drives.append(drive);
        }
    } while (result != kCFURLEnumeratorEnd);

    CFRelease(enumerator);
    return drives;
}

QDriveInfo QDriveInfoPrivate::rootDrive()
{
    return QDriveInfo(QLatin1String("/"));
}

QT_END_NAMESPACE
