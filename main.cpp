#include <QtCore/QCoreApplication>

#include "qdriveinfo.h"
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QTimer>

void f()
{
    QDriveInfo info;
    QDriveInfo info2(info);
    info.setRootPath("/");
    qDebug() << info.rootPath();
    qDebug() << info2.rootPath();
    info2 = info;
    info = info2;
    qDebug() << info.rootPath();
    qDebug() << info2.rootPath();

    info.setRootPath("/Volumes/H/Downloads");
    qDebug() << info.isValid() << info.isReady();
    qDebug() << info.fileSystemName() << info.device()
            << info.availableSize() << info.freeSize() << info.totalSize()
            << info.name() << info.rootPath();
}
void dumpDriveInfoList(const QList<QDriveInfo> &drives, const QString &title)
{
    qDebug() << endl << "=====" << title << "=====";
    foreach (const QDriveInfo &drive, drives) {
        qDebug() << "==============";
        if (drive.isValid()) {
            if (drive.isReady()) {
                qDebug() << "   " << "rootPath:" << drive.rootPath();
                qDebug() << "   " << "name:" << drive.name();
                qDebug() << "   " << "fileSystemName:" << drive.fileSystemName();
                qDebug() << "   " << "device:" << drive.device();
                qDebug() << "   " << "size:" << drive.totalSize()/1000/1000 << "MB";
                qDebug() << "   " << "freeSize:" << drive.freeSize()/1000/1000 << "MB";
                qDebug() << "   " << "availableSize:" << drive.availableSize()/1000/1000 << "MB";
                switch(drive.type()) {
                case QDriveInfo::InvalidDrive:
                    qDebug() << "   " << "QDrive::InvalidDrive"; break;
                case QDriveInfo::InternalDrive:
                    qDebug() << "   " << "QDrive::InternalDrive"; break;
                case QDriveInfo::RemovableDrive:
                    qDebug() << "   " << "QDrive::RemovableDrive"; break;
                case QDriveInfo::RemoteDrive:
                    qDebug() << "   " << "QDrive::RemoteDrive"; break;
                case QDriveInfo::CdromDrive:
                    qDebug() << "   " << "QDrive::CdromDrive"; break;
                case QDriveInfo::InternalFlashDrive:
                    qDebug() << "   " << "QDrive::InternalFlashDrive"; break;
                case QDriveInfo::RamDrive:
                    qDebug() << "   " << "QDrive::RamDrive"; break;
                }
            } else {
                qDebug() << "    " <<  drive.rootPath() << "is not ready";
            }
        } else {
            qDebug() << "    " <<  drive.rootPath() << "is not valid";
        }

    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    dumpDriveInfoList(QDriveInfo::drives(), "QDriveInfo::drives()");

    QList<QDriveInfo> invalidDrives = QList<QDriveInfo>()
        << QDriveInfo() << QDriveInfo("") << QDriveInfo("invalid|path");
    dumpDriveInfoList(invalidDrives, "Invalid Drives");

#if defined(Q_OS_MAC)
    QList<QDriveInfo> macDrives = QList<QDriveInfo>()
        << QDriveInfo("/") << QDriveInfo("/Volumes/H/Downloads");
    dumpDriveInfoList(macDrives, "Mac Drives");
#endif

    QTimer::singleShot(1000, &a, SLOT(quit()));

    return a.exec();
}
