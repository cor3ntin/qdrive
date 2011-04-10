#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

#include <QDriveInfo>
#include <QTimer>

#include <QDriveController>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QDriveController c;

    // Windows:
//    qDebug() << "QDriveController::mount" << c.mount("E:\\", "C:\\1");
//    qDebug() << "QDriveController::unmount" << c.unmount("C:\\1");

    // Mac:
//    qDebug() << "QDriveController::mount" << c.mount("/dev/disk1", "");
//    qDebug() << "QDriveController::unmount" << c.unmount("/Volumes/NO NAME");
//    qDebug() << "QDriveController::eject" << c.eject("/Volumes/NO NAME");

//    c.mount("/dev/disk0s3", "/Volumes/1");
//    c.mount("/Volumes/Data HD/Images/openSUSE-11.4-DVD-x86_64.iso", "/Volumes/1");
//    c.unmount("/dev/disk1");

    foreach (const QDriveInfo &drive, QDriveInfo::drives()) {
        qDebug() << "==============";
        if (drive.isValid()) {
            if (drive.isReady()) {
                qDebug() << drive.rootPath();
                if (drive.isRoot())
                    qDebug() << "   " << "isRoot:" << drive.isRoot();
                if (drive.isReadOnly())
                    qDebug() << "   " << "isReadOnly:" << drive.isReadOnly();
                qDebug() << "   " << "name:" << drive.name();
                qDebug() << "   " << "fileSystemName:" << drive.fileSystemName();
                qDebug() << "   " << "device:" << drive.device();
                qDebug() << "   " << "size:" << drive.bytesTotal()/1000/1000 << "MB";
                qDebug() << "   " << "freeSize:" << drive.bytesFree()/1000/1000 << "MB";
                qDebug() << "   " << "availableSize:" << drive.bytesAvailable()/1000/1000 << "MB";
                switch(drive.type()) {
                case QDriveInfo::InvalidDrive:
                    qDebug() << "   " << "type:" << "QDriveInfo::InvalidDrive";
                    break;
                case QDriveInfo::InternalDrive:
                    qDebug() << "   " << "type:" << "QDriveInfo::InternalDrive";
                    break;
                case QDriveInfo::RemovableDrive:
                    qDebug() << "   " << "type:" << "QDriveInfo::RemovableDrive";
                    break;
                case QDriveInfo::RemoteDrive:
                    qDebug() << "   " << "type:" << "QDriveInfo::RemoteDrive";
                    break;
                case QDriveInfo::CdromDrive:
                    qDebug() << "   " << "type:" << "QDriveInfo::CdromDrive";
                    break;
                case QDriveInfo::InternalFlashDrive:
                    qDebug() << "   " << "type:" << "QDriveInfo::InternalFlashDrive";
                    break;
                case QDriveInfo::RamDrive:
                    qDebug() << "   " << "type:" << "QDriveInfo::RamDrive";
                    break;
                default:
                    break;
                }
            } else {
                qDebug() << "    " <<  drive.rootPath() << "is not ready";
            }
        } else {
            qDebug() << "    " <<  drive.rootPath() << "is not valid";
        }
    }

    QTimer::singleShot(15000, &app, SLOT(quit()));
    return app.exec();
}
