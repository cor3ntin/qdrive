#include "qdrivecontroller_p.h"

#include <QtCore/QStringList>

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
