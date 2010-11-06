#ifndef QDRIVE_LINUX_P_H
#define QDRIVE_LINUX_P_H

#include "qdrive_p.h"

#define MOUNTED "/etc/mtab"
#define DISK_BY_LABEL "/dev/disk/by-label"
class QDrivePrivate : public QDrivePrivateBase
{
public:
    QDrivePrivate();

    void stat(uint requiredFlags);
    void statFS();
    void getMountEntry();
    void getType();
    void getName();
    QDrive::DriveType determineType();
};

#endif // QDRIVE_LINUX_P_H
