#ifndef QDRIVE_MAC_P_H
#define QDRIVE_MAC_P_H

#include "qdrive_p.h"

class QDrivePrivate: public QDrivePrivateBase
{
public:
    QDrivePrivate();

    void stat(uint requiredFlags);
    void statFS();
    void getVolumeInfo();
    void getType();
    QDrive::DriveType determineType();
};

#endif // QDRIVE_MAC_P_H
