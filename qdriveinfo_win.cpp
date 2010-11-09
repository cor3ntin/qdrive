#include "qdriveinfo.h"
#include "qdriveinfo_p.h"

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QDebug>

#include "qplatformdefs.h"
#include <WinBase.h>
#include <Windows.h>

QStringList QDriveInfoPrivate::drivePaths()
{
    QStringList ret;
    quint32 driveBits = (quint32) GetLogicalDrives() & 0x3ffffff;
    char driveName[] = "A:/";
    while (driveBits) {
        if (driveBits & 1)
            ret.append(QLatin1String(driveName));
        driveName[0]++;
        driveBits = driveBits >> 1;
    }
    return ret;
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
    bitmask = CachedReadyFlag | CachedNameFlag | CachedFileSystemNameFlag;
    if (requiredFlags & bitmask) {
        getVolumeInformation();
        data->setCachedFlag(bitmask);
    }

    bitmask = CachedAvailableSizeFlag | CachedFreeSizeFlag | CachedSizeFlag;
    if (requiredFlags & bitmask) {
        getDiskFreeSpace();
        data->setCachedFlag(bitmask);
    }

    bitmask = CachedDeviceFlag;
    if (requiredFlags & bitmask) {
        getDevice();
        data->setCachedFlag(bitmask);
    }

    bitmask = CachedTypeFlag;
    if (requiredFlags & bitmask) {
        getType();
        data->setCachedFlag(bitmask);
    }
}

void QDriveInfoPrivate::getVolumeInformation()
{
    wchar_t nameArr[MAX_PATH] = L"";
    wchar_t fileSystemNameArr[MAX_PATH] = L"";
    bool result;

    result = GetVolumeInformation((WCHAR *)data->rootPath.utf16(),
                                  nameArr, MAX_PATH*sizeof(wchar_t),
                                  0,
                                  0,
                                  0,
                                  fileSystemNameArr, MAX_PATH*sizeof(wchar_t)
                                  );
    if (!result) {
        DWORD error = GetLastError();
        if (error == ERROR_NOT_READY)
            data->ready = false;
//        qDebug() << "error" << error;
        return;
    }

    data->ready = true;
    data->name = QString::fromWCharArray(nameArr);
    data->fileSystemName = QString::fromWCharArray(fileSystemNameArr);
}

void QDriveInfoPrivate::getDiskFreeSpace()
{
//    qDebug("getDiskFreeSpace");
    bool result;

    result = GetDiskFreeSpaceEx((WCHAR *)data->rootPath.utf16(),
                                (PULARGE_INTEGER)&(data->availableSize),
                                (PULARGE_INTEGER)&(data->totalSize),
                                (PULARGE_INTEGER)&(data->freeSize)
                                );
    if (!result) {
//        DWORD error = GetLastError();
//        if (error == ERROR_NOT_READY)
//            ready = false;
//        qDebug() << "error" << error;
        return;
    }
}

void QDriveInfoPrivate::getDevice()
{
//    wchar_t rootPathName[MAX_PATH] = L"";
//    rootPath.toWCharArray(rootPathName);

    wchar_t deviceBuffer[MAX_PATH] = L"";
    bool result;

    result = GetVolumeNameForVolumeMountPoint((WCHAR *)data->rootPath.utf16(),
                                              deviceBuffer,
                                              MAX_PATH);
    if (!result) {
//        DWORD error = GetLastError();
        return;
    }

    data->device = QString::fromWCharArray(deviceBuffer);
}

void QDriveInfoPrivate::getType()
{
    data->type = determineType();
}

QDriveInfo::DriveType QDriveInfoPrivate::determineType()
{
#if !defined(Q_OS_WINCE)
    uint result = GetDriveType((WCHAR *)data->rootPath.utf16());
    switch (result) {
    case 0:
    case 1:
        return QDriveInfo::NoDrive;
        break;
    case 2:
        return QDriveInfo::RemovableDrive;
        break;
    case 3:
        return QDriveInfo::InternalDrive;
        break;
    case 4:
        return QDriveInfo::RemoteDrive;
        break;
    case 5:
        return QDriveInfo::CdromDrive;
        break;
    case 6:
        break;
    };
#endif
    return QDriveInfo::NoDrive;
}
