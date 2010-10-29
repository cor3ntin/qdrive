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
    bool setName(const QString &name);
    void getType();
};

#endif // QDRIVE_MAC_P_H
