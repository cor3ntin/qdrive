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
        } else {
            qDebug() << "    " <<  f->rootPath() << "is not ready";
        }

    }

//    foreach(QFileInfo f, QDir::drives()) {
//     qDebug() << "FileInfo absolute file path" << f.absoluteFilePath();
//    }
    return a.exec();
}
