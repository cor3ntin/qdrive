#ifndef QDRIVECONTROLLER_P_H
#define QDRIVECONTROLLER_P_H

#include "qdrivecontroller.h"

#include <QtCore/QAtomicInt>
#include <QtCore/QObject>

#include "qsystemerror_p.h"

class QDriveControllerPrivate
{
public:
//    QDriveController::MountError error;
//    QString errorString;

    QSystemError error;
    void setLastError(int errorCode);
};


class QDriveWatcherEngine;

class QDriveWatcher : public QObject
{
    Q_OBJECT

public:
    explicit QDriveWatcher(QObject *parent = 0);
    ~QDriveWatcher();

    void start();
    void stop();

    inline void emitDriveAdded(const QString &driveOrPath)
    { emit driveAdded(driveOrPath); }
    inline void emitDriveRemoved(const QString &driveOrPath)
    { emit driveRemoved(driveOrPath); }

Q_SIGNALS:
    void driveAdded(const QString &path);
    void driveRemoved(const QString &path);

protected:
    bool start_sys();
    void stop_sys();

private:
    QAtomicInt startStopCounter;
    QDriveWatcherEngine *engine;
};


#if defined(Q_OS_MAC)

#include <QtCore/QSet>
#include <QtCore/QThread>

#include <DiskArbitration/DiskArbitration.h>

class QDriveWatcherEngine : public QThread
{
    Q_OBJECT

public:
    QDriveWatcherEngine();
    ~QDriveWatcherEngine();

    void stop();

    void addDrive(const QString &path);
    void removeDrive(const QString &path);
    void updateDrives();

protected:
    void run();

Q_SIGNALS:
    void driveAdded(const QString &path);
    void driveRemoved(const QString &path);

private:
    void populateVolumes();

    volatile bool m_running;

    DASessionRef m_session;
    QSet<QString> volumes;
};

#elif defined(Q_OS_SYMBIAN)

#include <e32base.h>
#include <f32file.h>

class QDriveWatcherEngine : public CActive
{
public:
    QDriveWatcherEngine(QDriveWatcher *watcher);
    ~QDriveWatcherEngine();

protected:  //from CActive
    void DoCancel();
    void RunL();

private:
    QDriveWatcher *m_watcher;
};

#elif defined(Q_OS_LINUX)

#include <QtCore/QSet>

class QDriveWatcherEngine : public QObject
{
    Q_OBJECT

public:
    QDriveWatcherEngine(QObject *parent);
    ~QDriveWatcherEngine();

    inline bool isValid() const
    { return mtabWatchA > 0; }

Q_SIGNALS:
    void driveAdded(const QString &path);
    void driveRemoved(const QString &path);

private Q_SLOTS:
    void deviceChanged();
    void inotifyActivated();

private:
    QSet<QString> drives;
    int inotifyFD;
    int mtabWatchA;
};

#endif

#endif // QDRIVECONTROLLER_P_H
