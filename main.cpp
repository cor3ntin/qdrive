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

    info.setRootPath("/Volumes/Macintosh HD");
    qDebug() << info.fileSystemName() << info.device();
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

//    f();
//    return a.exec();

//    qDebug() << QDrive::drivePaths();
    foreach (QDriveInfo drive, QDriveInfo::drives()) {
//        qDebug() << "=======";
        QDriveInfo *f = &drive;
        if (f->ready()) {
            qDebug() << "   " << "rootPath:" << f->rootPath();
            qDebug() << "   " << "name:" << f->name();
            qDebug() << "   " << "fileSystemName:" << f->fileSystemName();
            qDebug() << "   " << "device:" << f->device();
            qDebug() << "   " << "size:" << f->totalSize()/1000/1000 << "MB";
            qDebug() << "   " << "freeSize:" << f->freeSize()/1000/1000 << "MB";
            qDebug() << "   " << "availableSize:" << f->availableSize()/1000/1000 << "MB";
            switch(f->type()) {
            case QDriveInfo::NoDrive:
                qDebug() << "   " << "QDrive::NoDrive"; break;
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
            qDebug() << "    " <<  f->rootPath() << "is not ready";
        }

    }

//    foreach(QFileInfo f, QDir::drives()) {
//     qDebug() << "FileInfo absolute file path" << f.absoluteFilePath();
//    }
    QTimer::singleShot(1000, &a, SLOT(quit()));
    return a.exec();
}
