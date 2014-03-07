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

#include <QtCore/QDir>
#include <QtCore/QVarLengthArray>

#include <userenv.h>

QT_BEGIN_NAMESPACE

void QDriveInfoPrivate::initRootPath()
{
    if (rootPath.isEmpty())
        return;

    QString path = QDir::toNativeSeparators(rootPath);
    rootPath.clear();

    if (path.startsWith(QLatin1String("\\\\?\\")))
        path = path.mid(4);
    if (path.isEmpty())
        return;
    path[0] = path[0].toUpper();
    if (!(path.length() >= 2
          && path.at(0).unicode() >= 'A' && path.at(0).unicode() <= 'Z'
          && path.at(1) == QLatin1Char(':'))) {
        return;
    }
    if (!path.endsWith(QLatin1Char('\\')))
        path.append(QLatin1Char('\\'));

    // ### test if disk mounted to folder on other disk
    wchar_t buffer[MAX_PATH + 1];
    if (::GetVolumePathName(reinterpret_cast<const wchar_t *>(path.utf16()), buffer, MAX_PATH))
        rootPath = QDir::fromNativeSeparators(QString::fromWCharArray(buffer));
}

static inline QByteArray getDevice(const QString &rootPath)
{
    const QString path = QDir::toNativeSeparators(rootPath);
    wchar_t deviceBuffer[MAX_PATH + 1];
    if (::GetVolumeNameForVolumeMountPoint(reinterpret_cast<const wchar_t *>(path.utf16()), deviceBuffer, MAX_PATH))
        return QString::fromWCharArray(deviceBuffer).toLatin1();

    return QByteArray();
}

static inline QDriveInfo::DriveType determineType(const QString &rootPath)
{
#if !defined(Q_OS_WINCE)
    UINT result = ::GetDriveType(reinterpret_cast<const wchar_t *>(rootPath.utf16()));
    switch (result) {
    case DRIVE_REMOVABLE: return QDriveInfo::RemovableDrive;
    case DRIVE_FIXED: return QDriveInfo::InternalDrive;
    case DRIVE_REMOTE: return QDriveInfo::RemoteDrive;
    case DRIVE_CDROM: return QDriveInfo::CdromDrive;
    case DRIVE_RAMDISK: return QDriveInfo::RamDrive;
    case DRIVE_UNKNOWN:
    case DRIVE_NO_ROOT_DIR:
    // fall through
    default:
        break;
    };
#else
    Q_UNUSED(rootPath)
#endif
    return QDriveInfo::UnknownDrive;
}

void QDriveInfoPrivate::doStat(uint requiredFlags)
{
    if (getCachedFlag(requiredFlags))
        return;

    if (!getCachedFlag(CachedRootPathFlag)) {
        initRootPath();
        setCachedFlag(CachedRootPathFlag);
    }

    if (rootPath.isEmpty() || (getCachedFlag(CachedValidFlag) && !valid))
        return;

    if (!getCachedFlag(CachedValidFlag))
        requiredFlags |= CachedValidFlag; // force drive validation


    uint bitmask = 0;

    bitmask = CachedFileSystemNameFlag | CachedNameFlag |
              CachedReadOnlyFlag | CachedReadyFlag | CachedValidFlag;
    if (requiredFlags & bitmask) {
        getVolumeInfo();
        if (valid && !ready)
            bitmask = CachedValidFlag;
        setCachedFlag(bitmask);

        if (!valid)
            return;
    }

    bitmask = CachedDeviceFlag;
    if (requiredFlags & bitmask) {
        device = getDevice(rootPath);
        setCachedFlag(bitmask);
    }

    bitmask = CachedBytesTotalFlag | CachedBytesFreeFlag | CachedBytesAvailableFlag;
    if (requiredFlags & bitmask) {
        getDiskFreeSpace();
        setCachedFlag(bitmask);
    }

    bitmask = CachedTypeFlag;
    if (requiredFlags & bitmask) {
        type = determineType(rootPath);
        setCachedFlag(bitmask);
    }
}

void QDriveInfoPrivate::getVolumeInfo()
{
    UINT oldmode = ::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

    QString path = QDir::toNativeSeparators(rootPath);
    wchar_t nameBuf[MAX_PATH + 1];
    DWORD fileSystemFlags = 0;
    wchar_t fileSystemNameBuf[MAX_PATH + 1];
    bool result = ::GetVolumeInformation(reinterpret_cast<const wchar_t *>(path.utf16()),
                                         nameBuf, MAX_PATH,
                                         0, 0,
                                         &fileSystemFlags,
                                         fileSystemNameBuf, MAX_PATH);
    if (!result) {
        ready = false;
        valid = (::GetLastError() == ERROR_NOT_READY);
    } else {
        ready = true;
        valid = true;

        fileSystemName = QString::fromWCharArray(fileSystemNameBuf).toLatin1();
        name = QString::fromWCharArray(nameBuf);

        readOnly = (fileSystemFlags & FILE_READ_ONLY_VOLUME) != 0;

        capabilities = 0;
        if (fileSystemFlags & FILE_SUPPORTS_OBJECT_IDS) // ?
            capabilities |= QDriveInfo::SupportsPersistentIDs;
        if (fileSystemName.toLower() == "ntfs") // ###
            capabilities |= QDriveInfo::SupportsSymbolicLinks;
        if (fileSystemFlags & FILE_SUPPORTS_HARD_LINKS)
            capabilities |= QDriveInfo::SupportsHardLinks;
        if (fileSystemFlags & FILE_SUPPORTS_USN_JOURNAL) // ?
            capabilities |= QDriveInfo::SupportsJournaling;
        if (fileSystemFlags & FILE_SUPPORTS_SPARSE_FILES)
            capabilities |= QDriveInfo::SupportsSparseFiles;
        if (fileSystemFlags & FILE_CASE_SENSITIVE_SEARCH)
            capabilities |= QDriveInfo::SupportsCaseSensitiveNames;
        if (fileSystemFlags & FILE_CASE_PRESERVED_NAMES)
            capabilities |= QDriveInfo::SupportsCasePreservedNames;
    }

    ::SetErrorMode(oldmode);
}

void QDriveInfoPrivate::getDiskFreeSpace()
{
    UINT oldmode = ::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

    QString path = QDir::toNativeSeparators(rootPath);
    ::GetDiskFreeSpaceEx(reinterpret_cast<const wchar_t *>(path.utf16()),
                         (PULARGE_INTEGER)&bytesAvailable,
                         (PULARGE_INTEGER)&bytesTotal,
                         (PULARGE_INTEGER)&bytesFree);

    ::SetErrorMode(oldmode);
}

QList<QDriveInfo> QDriveInfoPrivate::drives()
{
    QList<QDriveInfo> drives;

    char driveName[] = "A:/";
    quint32 driveBits = quint32(::GetLogicalDrives()) & 0x3ffffff;
    while (driveBits) {
        if (driveBits & 1) {
            QDriveInfoPrivate data;
            data.rootPath = QString::fromLatin1(driveName);
            data.setCachedFlag(CachedRootPathFlag);
            drives.append(QDriveInfo(data));
        }
        driveName[0]++;
        driveBits = driveBits >> 1;
    }

    return drives;
}

QDriveInfo QDriveInfoPrivate::rootDrive()
{
    DWORD dwBufferSize = 128;
    QVarLengthArray<wchar_t, 128> profilesDirectory(dwBufferSize);
    bool ok;
    do {
        if (dwBufferSize > (DWORD)profilesDirectory.size())
            profilesDirectory.resize(dwBufferSize);
        ok = ::GetProfilesDirectory(profilesDirectory.data(), &dwBufferSize);
    } while (!ok && GetLastError() == ERROR_INSUFFICIENT_BUFFER);
    if (ok)
        return QDriveInfo(QString::fromWCharArray(profilesDirectory.data(), profilesDirectory.size()));

    return QDriveInfo();
}

QT_END_NAMESPACE
