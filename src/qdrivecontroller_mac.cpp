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

    QDriveWatcher *watcher = reinterpret_cast<QDriveWatcher*>(context);
    watcher->addDrive(path);
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
//    CFShow(DADiskCopyDescription(disk));
    QString path = getDiskPath(disk);
    QDriveWatcher *watcher = reinterpret_cast<QDriveWatcher*>(context);
    if (path.isEmpty()) {
        // if we didn't receive path from API, we maunally determine lost drive
        // (fixes bug with .dmg and .iso images
        watcher->updateDrives();

        return;
    }

    watcher->removeDrive(path);
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
    start();
}

QDriveWatcher::~QDriveWatcher()
{
    stop();
    DAUnregisterCallback(m_session, (void*)mountCallback1, this);
    DAUnregisterCallback(m_session, (void*)mountCallback2, this);
    DAUnregisterCallback(m_session, (void*)unmountCallback, this);
    qDebug("QDriveWatcher::~QDriveWatcher");
    CFRelease(m_session);
}

void QDriveWatcher::stop()
{
    m_running = false;
    wait();
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
    foreach (const QDriveInfo &info, QDriveInfo::drives()) {
        volumes.insert(info.rootPath());
    }
}

void QDriveWatcher::addDrive(const QString &path)
{
    if (!volumes.contains(path)) {
        volumes.insert(path);
        emit driveAdded(path);
    }
}

void QDriveWatcher::removeDrive(const QString &path)
{
    if (volumes.remove(path)) {
        emit driveRemoved(path);
    }
}

void QDriveWatcher::updateDrives()
{
    QSet<QString> oldDrives = volumes;

    volumes.clear();
    populateVolumes();

    foreach (const QString &path, oldDrives) {
        if (!volumes.contains(path)) {
            emit driveRemoved(path);
        }
    }

    foreach (const QString &path, volumes) {
        if (!oldDrives.contains(path)) {
            emit driveAdded(path);
        }
    }
}

void diskMountCallback(DADiskRef disk, DADissenterRef dissenter, void *context )
{
    qDebug() << "diskMountCallback";
    CFShow(disk);
    qDebug() << "dissenter";
    CFShow(dissenter);
}

bool QDriveController::mount(const QString &device, const QString &path)
{
    CFStringRef disk = CFStringCreateWithCharacters(kCFAllocatorDefault,
                                                    (const ushort*)device.data(),
                                                    device.length());
    CFShow(disk);
    FSVolumeRefNum refNum;
        CFURLRef url = CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault,
                                                               (const UInt8*)path.toUtf8().data(),
                                                               path.length(),
                                                               true);
    OSStatus result = FSMountLocalVolumeSync(disk, url, &refNum, kFSIterateReserved);
    qDebug() << result;
    if (result != noErr) {
        qDebug() << "failed";
    }
//    int              ret      = -1;
//    DASessionRef     session  = NULL;
//    DADiskRef        disk     = NULL;

//    // create a new Disk Arbitration session
//    session = DASessionCreate(kCFAllocatorDefault);
//    qDebug() << session << "failed to create Disk Arbitration session";
//    DASessionScheduleWithRunLoop(session, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

//    // create a new disk object from the given BSD device name
//    disk = DADiskCreateFromBSDName(kCFAllocatorDefault, session, device.toUtf8().data());
//    CFShow(disk);
//    qDebug() << disk << "failed to create disk object";

//    CFURLRef url = CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault,
//                                                           (const UInt8*)path.toUtf8().data(),
//                                                           path.length(),
//                                                           true);
//    CFShow(url);
//    DADiskMount(disk, url, kDADiskMountOptionDefault, diskMountCallback, this);
//    SInt32 result;
//    do {
//        result = CFRunLoopRunInMode(kCFRunLoopDefaultMode, TIME_INTERVAL, true);
//    } while (true && result);
}

bool QDriveController::unmount(const QString &device)
{
    DASessionRef     session  = NULL;
    DADiskRef        disk     = NULL;

    // create a new Disk Arbitration session
    session = DASessionCreate(kCFAllocatorDefault);
//    qDebug() << session << "failed to create Disk Arbitration session";
    DASessionScheduleWithRunLoop(session, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

    // create a new disk object from the given BSD device name
    disk = DADiskCreateFromBSDName(kCFAllocatorDefault, session, device.toUtf8().data());
    CFShow(disk);
//    qDebug() << disk << "failed to create disk object";

    DADiskUnmount(disk, kDADiskUnmountOptionDefault, diskMountCallback, this);
    SInt32 result;
    do {
        result = CFRunLoopRunInMode(kCFRunLoopDefaultMode, TIME_INTERVAL, true);
    } while (true && result);
}

