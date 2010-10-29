#include <QtCore/QCoreApplication>

#include "qdrive.h"
#include <QFileInfo>
#include <QDir>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);


    qDebug() << QDrive::drivePaths();
    foreach (QString drive, QDrive::drivePaths()) {
        qDebug() << "=======";
        QDrive *f = new QDrive(drive);
        if (f->ready()) {
            qDebug() << "   " << "rootPath:" << f->rootPath();
            qDebug() << "   " << "name:" << f->name();
            qDebug() << "   " << "fileSystemName:" << f->fileSystemName();
            qDebug() << "   " << "device:" << f->device();
            qDebug() << "   " << "size:" << f->size()/1000/1000 << "MB";
            qDebug() << "   " << "freeSize:" << f->freeSize()/1000/1000 << "MB";
            qDebug() << "   " << "availableSize:" << f->availableSize()/1000/1000 << "MB";
            switch(f->type()) {
            case QDrive::NoDrive:
                qDebug() << "   " << "QDrive::NoDrive"; break;
            case QDrive::InternalDrive:
                qDebug() << "   " << "QDrive::InternalDrive"; break;
            case QDrive::RemovableDrive:
                qDebug() << "   " << "QDrive::RemovableDrive"; break;
            case QDrive::RemoteDrive:
                qDebug() << "   " << "QDrive::RemoteDrive"; break;
            case QDrive::CdromDrive:
                qDebug() << "   " << "QDrive::CdromDrive"; break;
            case QDrive::InternalFlashDrive:
                qDebug() << "   " << "QDrive::InternalFlashDrive"; break;
            case QDrive::RamDrive:
                qDebug() << "   " << "QDrive::RamDrive"; break;
            }
        } else {
            qDebug() << "    " <<  f->rootPath() << "is not ready";
        }

    }

//    foreach(QFileInfo f, QDir::drives()) {
//     qDebug() << "FileInfo absolute file path" << f.absoluteFilePath();
//    }
    return a.exec();
}
