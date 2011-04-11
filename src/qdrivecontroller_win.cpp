#include "qdrivecontroller.h"
#include "qdrivecontroller_p.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QStringList>
#include <QDriveInfo>

#define _WIN32_WINNT 0x0500
#include <qt_windows.h>
#include <dbt.h>

#ifndef DBT_CUSTOMEVENT
#  define DBT_CUSTOMEVENT 0x8006
#endif

#define QDRIVECONTROLLER_DEBUG 1

Q_CORE_EXPORT HINSTANCE qWinAppInst();

static inline QStringList drivesFromMask(quint32 driveBits)
{
        QStringList ret;

        char driveName[] = "A:/";
        driveBits = (driveBits & 0x3ffffff);
        while (driveBits) {
            if (driveBits & 0x1)
                ret.append(QString::fromLatin1(driveName));
            ++driveName[0];
            driveBits = driveBits >> 1;
        }

        return ret;
}

LRESULT CALLBACK dw_internal_proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_DEVICECHANGE) {
        DEV_BROADCAST_HDR *lpdb = (DEV_BROADCAST_HDR *)lParam;
        switch (wParam) {
        case DBT_DEVNODES_CHANGED:
#ifdef QDRIVECONTROLLER_DEBUG
            qWarning("DBT_DEVNODES_CHANGED message received, no extended info.");
#endif
            break;

        case DBT_QUERYCHANGECONFIG:
#ifdef QDRIVECONTROLLER_DEBUG
            qWarning("DBT_QUERYCHANGECONFIG message received, no extended info.");
#endif
            break;
        case DBT_CONFIGCHANGED:
#ifdef QDRIVECONTROLLER_DEBUG
            qWarning("DBT_CONFIGCHANGED message received, no extended info.");
#endif
            break;
        case DBT_CONFIGCHANGECANCELED:
#ifdef QDRIVECONTROLLER_DEBUG
            qWarning("DBT_CONFIGCHANGECANCELED message received, no extended info.");
#endif
            break;

        case DBT_DEVICEARRIVAL:
        case DBT_DEVICEQUERYREMOVE:
        case DBT_DEVICEQUERYREMOVEFAILED:
        case DBT_DEVICEREMOVEPENDING:
        case DBT_DEVICEREMOVECOMPLETE:
            if (lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME) {
                DEV_BROADCAST_VOLUME *db_volume = (DEV_BROADCAST_VOLUME *)lpdb;
                QStringList drives = drivesFromMask(db_volume->dbcv_unitmask);
#ifdef GWLP_USERDATA
                QDriveWatcher *watcher = (QDriveWatcher *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
#else
                QDriveWatcher *watcher = (QDriveWatcher *)GetWindowLong(hwnd, GWL_USERDATA);
#endif

                if (wParam == DBT_DEVICEARRIVAL) {
                    foreach (const QString &drive, drives) {
#ifdef QDRIVECONTROLLER_DEBUG
                        if (db_volume->dbcv_flags & DBTF_MEDIA)
                            qWarning("Drive %c: Media has been arrived.", drive.at(0).toAscii());
                        else if (db_volume->dbcv_flags & DBTF_NET)
                            qWarning("Drive %c: Network share has been mounted.", drive.at(0).toAscii());
                        else
                            qWarning("Drive %c: Device has been added.", drive.at(0).toAscii());
#endif
                        watcher->emitDriveAdded(drive);
                    }
                } else if (wParam == DBT_DEVICEQUERYREMOVE) {
                } else if (wParam == DBT_DEVICEQUERYREMOVEFAILED) {
                } else if (wParam == DBT_DEVICEREMOVEPENDING) {
                } else if (wParam == DBT_DEVICEREMOVECOMPLETE) {
                    foreach (const QString &drive, drives) {
#ifdef QDRIVECONTROLLER_DEBUG
                        if (db_volume->dbcv_flags & DBTF_MEDIA)
                            qWarning("Drive %c: Media has been removed.", drive.at(0).toAscii());
                        else if (db_volume->dbcv_flags & DBTF_NET)
                            qWarning("Drive %c: Network share has been unmounted.", drive.at(0).toAscii());
                        else
                            qWarning("Drive %c: Device has been removed.", drive.at(0).toAscii());
#endif
                        watcher->emitDriveRemoved(drive);
                    }
                }
            }
            break;
        case DBT_DEVICETYPESPECIFIC:
#ifdef QDRIVECONTROLLER_DEBUG
            qWarning("DBT_DEVICETYPESPECIFIC message received, may contain an extended info.");
#endif
            break;
        case DBT_CUSTOMEVENT:
#ifdef QDRIVECONTROLLER_DEBUG
            qWarning("DBT_CUSTOMEVENT message received, contains an extended info.");
#endif
            break;
        case DBT_USERDEFINED:
#ifdef QDRIVECONTROLLER_DEBUG
            qWarning("WM_DEVICECHANGE user defined message received, can not handle.");
#endif
            break;

        default:
#ifdef QDRIVECONTROLLER_DEBUG
            qWarning("WM_DEVICECHANGE message received, unhandled value %d.", wParam);
#endif
            break;
        }
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

static inline QString className()
{
    return QLatin1String("QDriveWatcherWin32_Internal_Widget") + QString::number(quintptr(dw_internal_proc));
}

static inline HWND dw_create_internal_window(const void* userData)
{
    HINSTANCE hi = qWinAppInst();

    WNDCLASS wc;
    wc.style = 0;
    wc.lpfnWndProc = dw_internal_proc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hi;
    wc.hIcon = 0;
    wc.hCursor = 0;
    wc.hbrBackground = 0;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = reinterpret_cast<const wchar_t *>(className().utf16());
    RegisterClass(&wc);

    HWND hwnd = CreateWindow(wc.lpszClassName,       // classname
                             wc.lpszClassName,       // window name
                             0,                      // style
                             0, 0, 0, 0,             // geometry
                             0,                      // parent
                             0,                      // menu handle
                             hi,                     // application
                             0);                     // windows creation data.
    if (!hwnd) {
        qWarning("QDriveWatcherEngine: Failed to create internal window: %d", (int)GetLastError());
    } else if (userData) {
#ifdef GWLP_USERDATA
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)userData);
#else
        SetWindowLong(hwnd, GWL_USERDATA, (LONG)userData);
#endif
    }

    return hwnd;
}

static inline void dw_destroy_internal_window(HWND hwnd)
{
    if (hwnd)
        DestroyWindow(hwnd);

    UnregisterClass(reinterpret_cast<const wchar_t *>(className().utf16()), qWinAppInst());
}


class QDriveWatcherEngine
{
public:
    HWND hwnd;
};


bool QDriveWatcher::start_sys()
{
    engine = new QDriveWatcherEngine;
    engine->hwnd = dw_create_internal_window(this);
    return engine->hwnd;
}

void QDriveWatcher::stop_sys()
{
    if (engine) {
        dw_destroy_internal_window(engine->hwnd);
        delete engine;
        engine = 0;
    }
}

#include <QDebug>

bool QDriveController::mount(const QString &device, const QString &path)
{
    QString targetPath = QDir::toNativeSeparators(path);
//    if (!targetPath.endsWith('\\'))
//        targetPath.append('\\');

    if (device.startsWith(QLatin1String("\\\\?\\"))) { // GUID
        qDebug() << "mounting by uid";

        bool result = SetVolumeMountPoint((wchar_t*)targetPath.utf16(), (wchar_t*)device.utf16());
        if (!result) {
            // TODO: add error handling
            qDebug() << "can't mount" << GetLastError();
            return false;
        }
    } else if (device.startsWith(QLatin1String("\\\\"))) { // network share
        qDebug() << "mounting share";

        NETRESOURCE resource;
        resource.dwType = RESOURCETYPE_ANY;
        resource.lpRemoteName = (wchar_t*)device.utf16();
        resource.lpLocalName = (wchar_t*)targetPath.utf16();
        resource.lpProvider = NULL;

        DWORD result = WNetAddConnection2(&resource, 0, 0, CONNECT_UPDATE_PROFILE);
        if (result != NO_ERROR) {
            qDebug() << "error mounting share:" << result;
            switch (result) {
            case ERROR_ACCESS_DENIED : qDebug() << "Access denied"; break;
            case ERROR_ALREADY_ASSIGNED : qDebug() << "ERROR_ALREADY_ASSIGNED"; break;
            case ERROR_BAD_DEV_TYPE : qDebug() << "ERROR_BAD_DEV_TYPE"; break;
            case ERROR_BAD_DEVICE : qDebug() << "ERROR_BAD_DEVICE"; break;
            case ERROR_BAD_NET_NAME : qDebug() << "ERROR_BAD_NET_NAME"; break;
            case ERROR_BAD_PROFILE : qDebug() << "ERROR_BAD_PROFILE"; break;
            case ERROR_BAD_PROVIDER : qDebug() << "ERROR_BAD_PROVIDER"; break;
            case ERROR_BAD_USERNAME : qDebug() << "ERROR_BAD_USERNAME"; break;
            case ERROR_BUSY : qDebug() << "ERROR_BUSY"; break;
            case ERROR_CANCELLED : qDebug() << "ERROR_CANCELLED"; break;
            case ERROR_CANNOT_OPEN_PROFILE : qDebug() << "ERROR_CANNOT_OPEN_PROFILE"; break;
            case ERROR_DEVICE_ALREADY_REMEMBERED : qDebug() << "ERROR_DEVICE_ALREADY_REMEMBERED"; break;
            case ERROR_EXTENDED_ERROR : qDebug() << "ERROR_EXTENDED_ERROR"; break;
            case ERROR_INVALID_ADDRESS : qDebug() << "ERROR_INVALID_ADDRESS"; break;
            case ERROR_INVALID_PARAMETER : qDebug() << "ERROR_INVALID_PARAMETER"; break;
            case ERROR_INVALID_PASSWORD : qDebug() << "ERROR_INVALID_PASSWORD"; break;
            case ERROR_LOGON_FAILURE : qDebug() << "ERROR_LOGON_FAILURE"; break;
            case ERROR_NO_NET_OR_BAD_PATH : qDebug() << "ERROR_NO_NET_OR_BAD_PATH"; break;
            case ERROR_NO_NETWORK : qDebug() << "ERROR_NO_NETWORK"; break;
            default: qDebug() << "other error"; break;
            }
            return false;
        }
    } else {
        if (QFileInfo(device).isDir()) {
            qDebug() << "mounting by path";
            QDriveInfo driveInfo(device);
            QString guid = driveInfo.device();
            return mount(guid, targetPath);
        }
        return false;
    }

    return true;
}

bool QDriveController::unmount(const QString &path)
{
    QString targetPath = QDir::toNativeSeparators(path);
    if (targetPath.startsWith(QLatin1String("\\\\")) || QDriveInfo(path).type() == QDriveInfo::RemoteDrive) { // share
        if (targetPath.endsWith(QLatin1Char('\\')))
            targetPath.chop(1);

        DWORD result = WNetCancelConnection2((wchar_t*)targetPath.utf16(), CONNECT_UPDATE_PROFILE, true);
        if (result != NO_ERROR) {
            qDebug() << "error unmounting share:" << result;
            switch (result) {
            case ERROR_BAD_PROFILE : qDebug() << "ERROR_BAD_PROFILE"; break;
            case ERROR_CANNOT_OPEN_PROFILE : qDebug() << "ERROR_CANNOT_OPEN_PROFILE"; break;
            case ERROR_DEVICE_IN_USE : qDebug() << "ERROR_DEVICE_IN_USE"; break;
            case ERROR_EXTENDED_ERROR : qDebug() << "ERROR_EXTENDED_ERROR"; break;
            case ERROR_NOT_CONNECTED : qDebug() << "ERROR_NOT_CONNECTED"; break;
            case ERROR_OPEN_FILES : qDebug() << "ERROR_OPEN_FILES"; break;
            default: qDebug() << "other error"; break;
            }
            return false;
        }
    } else {
        if (!targetPath.endsWith(QLatin1Char('\\')))
            targetPath.append(QLatin1Char('\\'));

        bool result = DeleteVolumeMountPoint((wchar_t*)targetPath.utf16());
        if (!result) {
            // TODO: add error handling
            qDebug() << "can't unmount" << GetLastError();
            return false;
        }
    }

    return true;
}
