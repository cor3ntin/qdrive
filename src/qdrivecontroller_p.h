#ifndef QDRIVECONTROLLER_P_H
#define QDRIVECONTROLLER_P_H

#include "qdrivecontroller.h"

#ifdef Q_OS_WIN
//#include <windows.h>
#define _WIN32_WINNT  0x0500
#include <qt_windows.h>
#include <dbt.h>
#endif

class DASessionThread;
class QDriveControllerPrivate
{
public:
    QDriveControllerPrivate();
    ~QDriveControllerPrivate();

    static DASessionThread *sessionThread;

#ifdef Q_OS_WIN
    HWND hwnd;
#endif
};

#ifdef Q_OS_MACX

#include <QtCore/QThread>
#include <QtCore/QSet>

#include <DiskArbitration/DiskArbitration.h>

#define TIME_INTERVAL 1

class DASessionThread : public QThread
{
    Q_OBJECT
public:
    explicit DASessionThread(QObject *parent = 0);
    ~DASessionThread();

    void stop();

    QSet<QString> volumes;

signals:
    void driveAdded(const QString &);

public slots:
    void testMount();

protected:
    void run();

private:
    DASessionRef session;
    volatile bool running;
};

#endif

//#ifdef Q_OS_LINUX

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

//#endif

#endif // QDRIVECONTROLLER_P_H
