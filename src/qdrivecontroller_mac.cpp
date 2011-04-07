#include "qdrivecontroller.h"
#include "qdrivecontroller_p.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>

#include <CoreServices/CoreServices.h>
#include <CoreFoundation/CFRunLoop.h>

#include <IOKit/storage/IOCDMedia.h>
#include <IOKit/storage/IODVDMedia.h>

#include <QDebug>
#include <QChar>

#include "qdriveinfo.h"

QString CFStringToQString(CFStringRef string)
{
    CFRange range = { 0, CFStringGetLength(string) };

    unsigned short *array = new unsigned short[range.length];
    CFStringGetCharacters(string, range, array);

    // TODO: remove copying and use QString::data()?
    QString result = QString::fromUtf16(array, range.length);

    delete [] array;
    return result;
}

QString getDiskPath(DADiskRef diskRef)
{
    CFDictionaryRef dictionary = DADiskCopyDescription(diskRef);
    if (!dictionary)
        return QString();

    CFURLRef url = (CFURLRef)CFDictionaryGetValue(dictionary, kDADiskDescriptionVolumePathKey);
    if (!url) {
        CFRelease(dictionary);
        return QString();
    }

    CFStringRef stringRef = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
    if (!stringRef) {
        CFRelease(dictionary);
        return QString();
    }

    QString path = CFStringToQString(stringRef);

    CFRelease(stringRef);
    CFRelease(dictionary);

    return path;
}

void checkNewDiskAndEmitSignal(DADiskRef disk, void *context)
{
    QString path = getDiskPath(disk);
    if (path.isEmpty())
        return;

    QDriveWatcher *sessionThread = reinterpret_cast<QDriveWatcher*>(context);
    sessionThread->addDrive(path);
}

// FIXME: here we get event when flash drive mounted
void mountCallback1(DADiskRef disk, CFArrayRef /*keys*/, void *context)
{
    checkNewDiskAndEmitSignal(disk, context);
}

// FIXME: here we get event when cdrom mounted? O_o
void mountCallback2(DADiskRef disk, void *context)
{
    DADiskRef wholeDisk = DADiskCopyWholeDisk(disk);
// only deal with whole disks here.. i.e. cdroms
    if (wholeDisk) {
        io_service_t mediaService;

        mediaService = DADiskCopyIOMedia(wholeDisk);
        if (mediaService) {
            if (IOObjectConformsTo(mediaService, kIOCDMediaClass)
                    || IOObjectConformsTo(mediaService, kIODVDMediaClass)) {

                checkNewDiskAndEmitSignal(disk, context);

            }
        }
        IOObjectRelease(mediaService);
        CFRelease(wholeDisk);
    }
}

void unmountCallback(DADiskRef disk, void *context)
{
    CFShow(DADiskCopyDescription(disk));
    QString path = getDiskPath(disk);
    QDriveWatcher *sessionThread = reinterpret_cast<QDriveWatcher*>(context);
    if (path.isEmpty()) {
        // if we didn't receive path from API, we maunally determine lost drive
        // (fixes bug with .dmg and .iso images
        sessionThread->updateDrives();
        return;
    }

    sessionThread->removeDrive(path);
}

QDriveWatcher::QDriveWatcher(QObject *parent) :
    QThread(parent),
    m_running(false)
{
    m_session = DASessionCreate(kCFAllocatorDefault);

    DARegisterDiskDescriptionChangedCallback(m_session,
                                             kDADiskDescriptionMatchVolumeMountable,
                                             kDADiskDescriptionWatchVolumePath,
                                             mountCallback1,
                                             this);

    DARegisterDiskAppearedCallback(m_session,
                                   kDADiskDescriptionMatchVolumeMountable,
                                   mountCallback2,
                                   this);

    DARegisterDiskDisappearedCallback(m_session,
                                      kDADiskDescriptionMatchVolumeMountable,
                                      unmountCallback,
                                      this);
    qDebug("starting thread");
    start();
}

QDriveWatcher::~QDriveWatcher()
{
    stop();
}

void QDriveWatcher::stop()
{
    m_running = false;
    wait();
    qDebug() << "stopped";
}

void QDriveWatcher::run()
{
    qDebug() << "DASessionThread::run" << currentThread();

    m_running = true;

    populateVolumes();

    DASessionScheduleWithRunLoop(m_session, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

    SInt32 result;
    do {
        result = CFRunLoopRunInMode(kCFRunLoopDefaultMode, TIME_INTERVAL, true);
    } while (m_running && result);

    DASessionUnscheduleFromRunLoop(m_session, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
}

void QDriveWatcher::populateVolumes()
{
    // TODO: get paths as StringList?
    foreach (QDriveInfo info, QDriveInfo::drives()) {
        volumes.insert(info.rootPath());
    }
}

void QDriveWatcher::addDrive(const QString &path)
{
    if (!volumes.contains(path)) {
        volumes.insert(path);
        qDebug() << "disk added - path:" << path;
        QMetaObject::invokeMethod(this, "driveAdded", Q_ARG(QString, path));
    }
}

void QDriveWatcher::removeDrive(const QString &path)
{
    if (volumes.remove(path)) {
        QMetaObject::invokeMethod(this, "driveRemoved", Q_ARG(QString, path));
    }
}

void QDriveWatcher::updateDrives()
{
    QSet<QString> oldDrives = volumes;

    volumes.clear();
    populateVolumes();

    foreach (const QString &path, oldDrives) {
        if (!volumes.contains(path)) {
            QMetaObject::invokeMethod(this, "driveRemoved", Q_ARG(QString, path));
        }
    }

    foreach (const QString &path, volumes) {
        if (!oldDrives.contains(path)) {
            QMetaObject::invokeMethod(this, "driveAdded", Q_ARG(QString, path));
        }
    }
}
