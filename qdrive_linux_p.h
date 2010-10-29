#ifndef QDRIVE_LINUX_P_H
#define QDRIVE_LINUX_P_H

#include "qdrive_p.h"

#define MOUNTED "/etc/mtab"

class QDrivePrivate : public QDrivePrivateBase
{
public:
    QDrivePrivate();

    void stat(uint requiredFlags);
    void statFS();
    void getMountEntry();
    bool setName(const QString &name);
    void getType();
};

#endif // QDRIVE_LINUX_P_H
