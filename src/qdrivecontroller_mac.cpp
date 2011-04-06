#include "qdrivecontroller.h"
#include "qdrivecontroller_p.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>

#include <CoreServices/CoreServices.h>
#include <CoreFoundation/CFRunLoop.h>

#include <QDebug>

DASessionThread *QDriveControllerPrivate::sessionThread = 0;

void diskDescriptionChangedCallback(DADiskRef disk, CFArrayRef /*keys*/, void *context)
{
    qDebug("diskDescriptionChangedCallback");
//    NSDictionary *batDoctionary;
//    batDoctionary = (NSDictionary *)DADiskCopyDescription(disk);
//    NSURL *volumePath = [[batDoctionary objectForKey:(NSString *)kDADiskDescriptionVolumePathKey] copy];

//    QString name = nsstringToQString([volumePath path]);

//    static_cast<QSystemStorageInfoPrivate*>(context)->storageChanged(true, name);
}

void diskAppearedCallback(DADiskRef diskRef, void *context)
{
    qDebug("diskAppearedCallback");
    qDebug() << "main thread:" << qApp->thread() << "current thread:" << QThread::currentThread();

    CFShow(diskRef);
    CFDictionaryRef dicrionary = DADiskCopyDescription(diskRef);
    if (!dicrionary)
        return;

    CFURLRef url = (CFURLRef)CFDictionaryGetValue(dicrionary, kDADiskDescriptionVolumePathKey);
    if (!url) {
        CFRelease(dicrionary);
        return; // here leaks dicrionary. TODO : fix it.
    }

    CFStringRef stringRef = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
    if (!stringRef) {
        CFRelease(dicrionary);
        return;
    }

    CFShow(stringRef);
    CFIndex length = CFStringGetLength(stringRef) + 1;
    char *volname = NewPtr(length);
    CFStringGetCString(stringRef, volname, length, kCFStringEncodingMacRoman);
    QString path = QFile::decodeName(volname);
    CFRelease(stringRef);
    DisposePtr(volname);

    DASessionThread *sessionThread = (DASessionThread*)context;
    if (!sessionThread->volumes.contains(path)) {
        sessionThread->volumes.insert(path);
        qDebug() << "path:" << path;
        QMetaObject::invokeMethod(sessionThread, "driveAdded", Q_ARG(QString, path));

        QMetaObject::invokeMethod(sessionThread, "testMount");
    }

    CFRelease(dicrionary);
}

void diskDisappearedCallback(DADiskRef disk, void *context)
{
    qDebug("diskDisappearedCallback");
//    NSDictionary *batDoctionary;
//    batDoctionary = (NSDictionary *)DADiskCopyDescription(disk);
//    NSURL *volumePath = [[batDoctionary objectForKey:(NSString *)kDADiskDescriptionVolumePathKey] copy];

//    QString name = nsstringToQString([volumePath path]);

//    static_cast<QSystemStorageInfoPrivate*>(context)->storageChanged(false,name);
}

DASessionThread::DASessionThread(QObject *parent) :
    QThread(parent),
    running(false)
{
    session = DASessionCreate(kCFAllocatorDefault);

    DARegisterDiskAppearedCallback(session,
                                   kDADiskDescriptionMatchVolumeMountable,
                                   diskAppearedCallback,
                                   this);

    DARegisterDiskDescriptionChangedCallback(session,
                                             kDADiskDescriptionMatchVolumeMountable,
                                             kDADiskDescriptionWatchVolumePath,
                                             diskDescriptionChangedCallback,
                                             this);

    DARegisterDiskDisappearedCallback(session,
                                      kDADiskDescriptionMatchVolumeMountable,
                                      diskDisappearedCallback,
                                      this);
    qDebug("starting thread");
    start();
}

DASessionThread::~DASessionThread()
{
    stop();
}

void DASessionThread::stop()
{
    running = false;
    wait();
    qDebug() << "stopped";
}

void DASessionThread::run()
{
    qDebug() << "DASessionThread::run" << currentThread();

    running = true;

    DASessionScheduleWithRunLoop(session, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

    SInt32 result;
    do {
        result = CFRunLoopRunInMode(kCFRunLoopDefaultMode, TIME_INTERVAL, true);
    } while (running && result);

    DASessionUnscheduleFromRunLoop(session, CFRunLoopGetCurrent(),kCFRunLoopDefaultMode);
}

void DASessionThread::testMount()
{
    qDebug() << "DASessionThread::testMount";
    qDebug() << qApp->thread() << currentThread();
}

QDriveControllerPrivate::QDriveControllerPrivate()
{
    if (!sessionThread) {
        // TODO: add mutexes
        sessionThread = new DASessionThread(qApp);
    }
}

QDriveControllerPrivate::~QDriveControllerPrivate()
{
}

