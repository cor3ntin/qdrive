#ifndef QDRIVECONTROLLER_P_H
#define QDRIVECONTROLLER_P_H

#include "qdrivecontroller.h"
#include <qglobal.h>

#include <QtCore/QSet>

#ifdef Q_OS_WIN
//#include <windows.h>
#define _WIN32_WINNT  0x0500
#include <qt_windows.h>
#include <dbt.h>
#endif

#ifdef Q_OS_MACX

#include <QtCore/QThread>
#include <QtCore/QSet>

#include <DiskArbitration/DiskArbitration.h>

#define TIME_INTERVAL 1

#endif

class QDriveWatcher;
class QDriveControllerPrivate
{
public:
    QDriveControllerPrivate();
    ~QDriveControllerPrivate();

    static QDriveWatcher *watcherInstance;

#ifdef Q_OS_WIN
    HWND hwnd;
#endif
};

#ifdef Q_OS_MACX
class QDriveWatcher : public QThread
#else
class QDriveWatcher : public QObject
#endif
{
    Q_OBJECT
public:
    explicit QDriveWatcher(QObject *parent = 0);
    ~QDriveWatcher();

#ifdef Q_OS_WIN
private:
    HWND hwnd;
#endif

#ifdef Q_OS_MACX
    void stop();

    QSet<QString> volumes; // careful, use ONLY from thread itself

protected:
    void run(); // from QThread

private:
    void populateVolumes();

    DASessionRef m_session;
    volatile bool m_running;
#endif

signals:
    void driveAdded(const QString &path);
    void driveRemoved(const QString &path);
};

//#endif

#ifdef Q_OS_LINUX

#include <QtCore/QSet>

class INotifyWatcher : public QObject
{
    Q_OBJECT
public:
    explicit INotifyWatcher(QObject *parent = 0);
    ~INotifyWatcher();

    static INotifyWatcher *watcher;

    void testMount(const QString &path);

private slots:
    void deviceChanged();
    void inotifyActivated();

private:
    int inotifyFD;
    int mtabWatchA;
    QSet<QString> drivePaths;
};

#endif

#ifdef Q_OS_SYMBIAN

#include <e32base.h>
#include <f32file.h>

class Watcher : public QObject, public CActive
{
    Q_OBJECT
public:
    explicit Watcher(QObject *parent = 0);
    ~Watcher();

    static Watcher *watcher;

protected:  //from CActive
    void DoCancel();
    void RunL();

};

#endif

#endif // QDRIVECONTROLLER_P_H
