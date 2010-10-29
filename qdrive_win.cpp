#include "qdrive_win_p.h"

//#define _WIN32_WINNT 0x0501
#include "qplatformdefs.h"
#include <WinBase.h>
#include <Windows.h>


#include <QDebug>
//================================== QDriveInfoPrivate ==================================

QDrivePrivate::QDrivePrivate()
{
}

void QDrivePrivate::stat(uint requiredFlags)
{
    uint bitmask = 0;

    bitmask = CachedReadyFlag | CachedNameFlag | CachedFileSystemNameFlag;
    if (requiredFlags & bitmask &&
            !getCachedFlag(bitmask))
        getVolumeInformation();

    bitmask = CachedAvailableSizeFlag | CachedFreeSizeFlag | CachedSizeFlag;
    if (requiredFlags & bitmask &&
        !getCachedFlag(bitmask))
        getDiskFreeSpace();

    bitmask = CachedDeviceFlag;
    if (requiredFlags & bitmask &&
        !getCachedFlag(bitmask))
        getDevice();

    bitmask = CachedTypeFlag;
    if (requiredFlags & bitmask &&
        !getCachedFlag(bitmask))
        getType();
}

void QDrivePrivate::getVolumeInformation()
{
    wchar_t rootPathName[MAX_PATH] = L"";
    rootPath.toWCharArray(rootPathName);

    wchar_t nameArr[MAX_PATH] = L"";
    wchar_t fileSystemNameArr[MAX_PATH] = L"";
    bool result;

    result = GetVolumeInformation(rootPathName,
                                  nameArr, MAX_PATH*sizeof(wchar_t),
                                  0,
                                  0,
                                  0,
                                  fileSystemNameArr, MAX_PATH*sizeof(wchar_t)
                                  );
    if (!result) {
        DWORD error = GetLastError();
        if (error == ERROR_NOT_READY)
            ready = false;
//        qDebug() << "error" << error;
        setCachedFlag(CachedReadyFlag);
        return;
    }

    ready = true;
    name = QString::fromWCharArray(nameArr);
    fileSystemName = QString::fromWCharArray(fileSystemNameArr);

    setCachedFlag(CachedReadyFlag | CachedNameFlag | CachedFileSystemNameFlag);
}

void QDrivePrivate::getDiskFreeSpace()
{
//    qDebug("getDiskFreeSpace");
    wchar_t rootPathName[MAX_PATH] = L"";
    rootPath.toWCharArray(rootPathName);

    bool result;

    result = GetDiskFreeSpaceEx(rootPathName,
                                (PULARGE_INTEGER)&availableSize,
                                (PULARGE_INTEGER)&size,
                                (PULARGE_INTEGER)&freeSize
                                );
    if (!result) {
//        DWORD error = GetLastError();
//        if (error == ERROR_NOT_READY)
//            ready = false;
//        qDebug() << "error" << error;
        setCachedFlag(CachedReadyFlag);
        return;
    }

    setCachedFlag(CachedAvailableSizeFlag | CachedFreeSizeFlag | CachedSizeFlag);
}

void QDrivePrivate::getDevice()
{
    wchar_t rootPathName[MAX_PATH] = L"";
    rootPath.toWCharArray(rootPathName);

    wchar_t buffer[MAX_PATH] = L"";
    bool result;

    result = GetVolumeNameForVolumeMountPoint(rootPathName,
                                              buffer,
                                              MAX_PATH);
    if (!result) {
//        DWORD error = GetLastError();
        return;
    }

    device = QString::fromWCharArray(buffer);

    setCachedFlag(CachedDeviceFlag);
}

bool QDrivePrivate::setName(const QString &name)
{
    bool result = SetVolumeLabel(reinterpret_cast<const WCHAR*>(rootPath.utf16()),
                                 reinterpret_cast<const WCHAR*>(name.utf16()));
    if (!result) {
        //error handling
        return false;
    }
    return true;
}

void QDrivePrivate::getType()
{
#if !defined(Q_OS_WINCE)
    uint result = GetDriveType((WCHAR *)rootPath.utf16());
    switch (result) {
    case 0:
    case 1:
        type = QDrive::NoDrive;
        break;
    case 2:
        type = QDrive::RemovableDrive;
        break;
    case 3:
        type = QDrive::InternalDrive;
        break;
    case 4:
        type = QDrive::RemoteDrive;
        break;
    case 5:
        type = QDrive::CdromDrive;
        break;
    case 6:
        break;
    };
#endif
    type = QDrive::NoDrive;

    setCachedFlag(CachedTypeFlag);
}

QStringList QDrive::drivePaths()
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
