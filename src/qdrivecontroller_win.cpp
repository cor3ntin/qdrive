#include "qdrivecontroller_p.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QStringList>
#include <QDriveInfo>

#ifndef DBT_CUSTOMEVENT
#  define DBT_CUSTOMEVENT 0x8006
#endif

// from Panter Commader
Q_CORE_EXPORT HINSTANCE qWinAppInst();

static QStringList drivesFromMask(quint32 driveBits)
{
        QStringList ret;

        char driveName[] = "A:/";
        driveBits = (driveBits & 0x3ffffff);
        while(driveBits)
        {
                if(driveBits & 0x1)
                        ret.append(QString::fromLatin1(driveName));
                ++driveName[0];
                driveBits = driveBits >> 1;
        }

        return ret;
}

LRESULT CALLBACK vw_internal_proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if(message == WM_DEVICECHANGE)
    {
        qDebug("WM_DEVICECHANGE");
        PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;
        switch(wParam)
        {
        case DBT_DEVNODES_CHANGED:
            qWarning("DBT_DEVNODES_CHANGED message received, no extended info.");
            break;

        case DBT_QUERYCHANGECONFIG:
            qWarning("DBT_QUERYCHANGECONFIG message received, no extended info.");
            break;
        case DBT_CONFIGCHANGED:
            qWarning("DBT_CONFIGCHANGED message received, no extended info.");
            break;
        case DBT_CONFIGCHANGECANCELED:
            qWarning("DBT_CONFIGCHANGECANCELED message received, no extended info.");
            break;

        case DBT_DEVICEARRIVAL:
        case DBT_DEVICEQUERYREMOVE:
        case DBT_DEVICEQUERYREMOVEFAILED:
        case DBT_DEVICEREMOVEPENDING:
        case DBT_DEVICEREMOVECOMPLETE:
            if(lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)
            {
                DEV_BROADCAST_VOLUME* db_volume = (DEV_BROADCAST_VOLUME*)lpdb;
                const QStringList& drives = drivesFromMask(db_volume->dbcv_unitmask);
                if(wParam == DBT_DEVICEARRIVAL)
                {
                    foreach(const QString &drive, drives)
                    {
                        if(db_volume->dbcv_flags & DBTF_MEDIA)
                            qWarning("Drive %c: Media has been arrived.", drive.at(0).toAscii());
                        else if(db_volume->dbcv_flags & DBTF_NET)
                            qWarning("Drive %c: Network share has been mounted.", drive.at(0).toAscii());
                        else
                            qWarning("Drive %c: Device has been added.", drive.at(0).toAscii());

                        QMetaObject::invokeMethod(QDriveControllerPrivate::watcherInstance,
                                                  "driveAdded",
                                                  Qt::QueuedConnection,
                                                  Q_ARG(QString, drive));
                    }
                }
                else if(wParam == DBT_DEVICEQUERYREMOVE)
                {
                }
                else if(wParam == DBT_DEVICEQUERYREMOVEFAILED)
                {
                }
                else if(wParam == DBT_DEVICEREMOVEPENDING)
                {
                }
                else if(wParam == DBT_DEVICEREMOVECOMPLETE)
                {
                    foreach(const QString &drive, drives)
                    {
                        if(db_volume->dbcv_flags & DBTF_MEDIA)
                            qWarning("Drive %c: Media has been removed.", drive.at(0).toAscii());
                        else if(db_volume->dbcv_flags & DBTF_NET)
                            qWarning("Drive %c: Network share has been unmounted.", drive.at(0).toAscii());
                        else
                            qWarning("Drive %c: Device has been removed.", drive.at(0).toAscii());

                        QMetaObject::invokeMethod(QDriveControllerPrivate::watcherInstance,
                                                  "driveRemoved",
                                                  Qt::QueuedConnection,
                                                  Q_ARG(QString, drive));
                    }
                }
            }
            break;
        case DBT_DEVICETYPESPECIFIC:
            qWarning("DBT_DEVICETYPESPECIFIC message received, can contain extended info.");
            break;
        case DBT_CUSTOMEVENT:
            qWarning("DBT_CUSTOMEVENT message received, contains extended info.");
            break;
        case DBT_USERDEFINED:
            qWarning("WM_DEVICECHANGE userdefined message received, can not handle.");
            break;
        default:
            qWarning("WM_DEVICECHANGE message received, unhandled value %d.", wParam);
            break;
        }
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

QString getClassName()
{
    return QLatin1String("VolumeWatcher_Internal_Widget") + QString::number(quintptr(vw_internal_proc));
}

static HWND vw_create_internal_window(const void* userData)
{
    QString className = getClassName();

    HINSTANCE hi = qWinAppInst();
    WNDCLASS wc;
    wc.style = 0;
    wc.lpfnWndProc = vw_internal_proc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hi;
    wc.hIcon = 0;
    wc.hCursor = 0;
    wc.hbrBackground = 0;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = reinterpret_cast<const wchar_t*>(className.utf16());
    RegisterClass(&wc);
    HWND wnd = CreateWindow(wc.lpszClassName,       // classname
                            wc.lpszClassName,       // window name
                            0,                      // style
                            0, 0, 0, 0,             // geometry
                            0,                      // parent
                            0,                      // menu handle
                            hi,                     // application
                            0);                     // windows creation data.
    if(!wnd)
    {
        qWarning("WindowsVolumeWatcherEngine: Failed to create internal window: %d", (int)GetLastError());
    }
    else if(userData)
    {
#ifdef GWLP_USERDATA
        SetWindowLongPtrA(wnd, GWLP_USERDATA, (LONG_PTR)userData);
#else
        SetWindowLongA(wnd, GWL_USERDATA, (LONG)userData);
#endif
    }
    return wnd;
}

static void vw_destroy_internal_window(HWND wnd)
{
    if(wnd)
        DestroyWindow(wnd);

    QString className = getClassName();

    UnregisterClass((wchar_t*)className.utf16(), qWinAppInst());
}


QDriveWatcher::QDriveWatcher(QObject *parent) :
    QObject(parent)
{
    hwnd = vw_create_internal_window(this);
}

QDriveWatcher::~QDriveWatcher()
{
    vw_destroy_internal_window(hwnd);
}

#include <QDebug>

bool QDriveController::mount(const QString &device, const QString &path)
{
    QString targetPath = QDir::toNativeSeparators(path);
//    if (!targetPath.endsWith('\\'))
//        targetPath.append('\\');

    bool result = false;
    if (device.startsWith(QLatin1String("\\\\?\\"))) { // GUID
        qDebug() << "mounting by uid";

        result = SetVolumeMountPoint((wchar_t*)targetPath.utf16(), (wchar_t*)device.utf16());

        if (!result) {
            // TODO: add error handling
            qDebug() << "can't mount" << GetLastError();
        }
    } else if (device.startsWith(QLatin1String("\\\\"))) { // network share
        qDebug() << "mounting share";
        NETRESOURCE source;
        DWORD result2;

        source.dwType = RESOURCETYPE_ANY;
        source.lpRemoteName = (wchar_t*)device.utf16();
        source.lpLocalName = (wchar_t*)targetPath.utf16();
        source.lpProvider = NULL;

        result2 = WNetAddConnection2(&source, 0, 0, CONNECT_UPDATE_PROFILE);

        if (result2 != NO_ERROR) {
            result = false;
            qDebug() << "error mounting share:" << result2;
            switch (result2) {
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
        }
        result = true;
    } else {
        if (QFileInfo(device).isDir()) {
            qDebug() << "mounting by path";
            QDriveInfo driveInfo(device);
            QString guid = driveInfo.device();
            result = mount(guid, targetPath);
        }
    }
    return result;
}

bool QDriveController::unmount(const QString &path)
{
    QDriveInfo driveInfo(path);
    if (path.startsWith("\\\\") || driveInfo.type() == QDriveInfo::RemoteDrive) { // share
        QString targetPath = QDir::toNativeSeparators(path);
        if (targetPath.endsWith('\\'))
            targetPath.chop(1);

        DWORD result;
        result = WNetCancelConnection2((wchar_t*)targetPath.utf16(), CONNECT_UPDATE_PROFILE, true);
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
        return true;
    } else {
        QString targetPath = QDir::toNativeSeparators(path);
        if (!targetPath.endsWith('\\'))
            targetPath.append('\\');

        bool result;
        result = DeleteVolumeMountPoint((wchar_t*)targetPath.utf16());
        if (!result) {
            // TODO: add error handling
            qDebug() << "can't unmount" << GetLastError();
        }
    }
    return false;
}

