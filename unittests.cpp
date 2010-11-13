#include <QtTest>

#include "qdriveinfo.h"
#include "unittestconfig.h"

class QDriveInfoTest: public QObject
{
    Q_OBJECT
private slots:
    void compareToConfig()
    {
        QDriveInfo info;
        QVERIFY(info.isValid() == false);
        QVERIFY(info.isReady() == false);
        info.setRootPath(CONSTRUCTOR);

        QVERIFY(info.isValid() == true);
        QVERIFY(info.isReady() == true);
        QVERIFY(info.rootPath() == QString::fromLocal8Bit(ROOT_PATH));
        QVERIFY(info.name() == QString::fromLocal8Bit(NAME));
        QVERIFY(info.device() == QString::fromLocal8Bit(DEVICE));
        QVERIFY(info.fileSystemName() == QString::fromLocal8Bit(FILESYSTEM));
        QVERIFY(info.type() == TYPE);
#if defined(Q_OS_MAC)
        QVERIFY(info.totalSize()/BLOCKSIZE == TOTALSIZE); // we devide onto block size
#endif
//        info.totalSize()
    }
};

QTEST_MAIN(QDriveInfoTest)
 #include "unittests.moc"
