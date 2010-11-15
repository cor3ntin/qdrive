#include <QtTest>

#include <QDriveInfo>
#include <QHash>
#include <QList>

struct DriveInfo
{
    QString rootPath;
    QByteArray device;
    QByteArray fileSystemName;
    QString name;

    QDriveInfo::DriveType type;
    uint capabilities;

    bool isReady;
    bool isValid;
    bool isRoot;
};

Q_DECLARE_METATYPE(DriveInfo)

Q_DECLARE_METATYPE(QDriveInfo)
Q_DECLARE_METATYPE(QDriveInfo::DriveType)


class tst_QDriveInfo: public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void common_data();

    void equals_data();
    void equals();

    void rootDrive();

    void constructor_data();
    void constructor();
    void setRootPath_data();
    void setRootPath();
    void refresh_data();
    void refresh();

    void drives_data();
    void drives();

private:
    QHash<QString, DriveInfo> testDrives;
};

void tst_QDriveInfo::initTestCase()
{
    qRegisterMetaType<DriveInfo>();
    qRegisterMetaType<QDriveInfo>();

    // invalid
    DriveInfo invalidDrive = { "", "", "", "", QDriveInfo::InvalidDrive, 0, false, false, false };
    testDrives.insert("invalid", invalidDrive);

#if defined(Q_OS_WIN)
    QDriveInfo::Capabilities caps_for_ntfs = (QDriveInfo::AccessControlListsSupport | QDriveInfo::HardlinksSupport);
    QDriveInfo::Capabilities caps_for_udf = QDriveInfo::ReadOnlyVolume;

    // local drives
    DriveInfo localDriveC = { "C:/", "\\\\?\\Volume{ddf26dc2-90e4-11df-acb1-806d6172696f}\\", "NTFS", "",
                              QDriveInfo::InternalDrive, caps_for_ntfs, true, true, true };
    testDrives.insert("C", localDriveC);
    DriveInfo localDriveD = { "D:/", "\\\\?\\Volume{e9204280-a0d1-11df-93bf-806d6172696f}\\", "NTFS", "old win",
                              QDriveInfo::InternalDrive, caps_for_ntfs, true, true, false };
    testDrives.insert("D", localDriveD);
    DriveInfo localDriveE = { "E:/", "\\\\?\\Volume{e920427e-a0d1-11df-93bf-806d6172696f}\\", "NTFS", "",
                              QDriveInfo::InternalDrive, caps_for_ntfs, true, true, false };
    testDrives.insert("E", localDriveE);
    DriveInfo localDriveF = { "F:/", "\\\\?\\Volume{e920427f-a0d1-11df-93bf-806d6172696f}\\", "NTFS", "",
                              QDriveInfo::InternalDrive, caps_for_ntfs, true, true, false };
    testDrives.insert("F", localDriveF);
    DriveInfo localDriveH = { "H:/", "\\\\?\\Volume{ddf26dc3-90e4-11df-acb1-806d6172696f}\\", "NTFS", "",
                              QDriveInfo::InternalDrive, caps_for_ntfs, true, true, false };
    testDrives.insert("H", localDriveH);

    // cdroms
    DriveInfo cdDriveI = { "I:/", "\\\\?\\Volume{2c15bbb2-bf3c-11df-8730-0004619ece73}\\", "UDF", "GRMCULF(X)RER(O)_EN-RU_DVD",
                              QDriveInfo::CdromDrive, caps_for_udf, true, true, false };
    testDrives.insert("I", cdDriveI);

    // flash drives
    DriveInfo localDriveL = { "L:/", "\\\\?\\Volume{6df0fd95-90d8-11df-977e-0004614df815}\\", "NTFS", "",
                              QDriveInfo::RemovableDrive, caps_for_ntfs, true, true, false };
    testDrives.insert("L", localDriveL);
#elif defined(Q_OS_MAC)
    QDriveInfo::Capabilities caps_for_hfs = (QDriveInfo::AccessControlListsSupport | QDriveInfo::HardlinksSupport);

    // local drives
    DriveInfo rootDrive = { "/", "/dev/disk0s2", "hfs", "Macintosh HD",
                            QDriveInfo::InternalDrive, caps_for_hfs, true, true, true };
    testDrives.insert("root", rootDrive);

    // net shares
    // ###
    DriveInfo netShare1 = { "/net", "", "", "",
                            QDriveInfo::RemoteDrive, 0, false, false, false };
    testDrives.insert("share1", netShare1);
#elif defined(Q_OS_LINUX)
    QDriveInfo::Capabilities caps_for_btrfs = (QDriveInfo::CaseSensitiveFileNames | QDriveInfo::AccessControlListsSupport |
                                               QDriveInfo::HardlinksSupport | QDriveInfo::SymlinksSupport);
    QDriveInfo::Capabilities caps_for_ext3 = (QDriveInfo::CaseSensitiveFileNames | QDriveInfo::AccessControlListsSupport |
                                              QDriveInfo::HardlinksSupport | QDriveInfo::SymlinksSupport);

    // local drives
    DriveInfo rootDrive = { "/", "/dev/sda2", "btrfs", "",
                            QDriveInfo::InternalDrive, caps_for_btrfs, true, true, true };
    testDrives.insert("sda2", rootDrive);
    DriveInfo bootDrive = { "/boot", "/dev/sda1", "ext3", "",
                            QDriveInfo::InternalDrive, caps_for_ext3, true, true, false };
    testDrives.insert("sda1", bootDrive);
#elif defined(Q_OS_SYMBIAN)
    // ###
#else
    // ###
#endif
}

void tst_QDriveInfo::common_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<DriveInfo>("driveInfo");

    QTest::newRow("invalid") << "" << testDrives["invalid"];
    QTest::newRow("invalid PP:/") << "" << testDrives["invalid"];

#if defined(Q_OS_WIN)
    QTest::newRow("c:\\") << "c:\\" << testDrives["C"];
    QTest::newRow("C:\\windows\\") << "C:\\windows\\" << testDrives["C"];
    QTest::newRow("C:\\windows\\..") << "C:\\windows\\.." << testDrives["C"];
    QTest::newRow("d:\\") << "d:\\" << testDrives["D"];
    QTest::newRow("e:\\") << "e:\\" << testDrives["E"];
    QTest::newRow("f:\\") << "f:\\" << testDrives["F"];
    QTest::newRow("h:\\") << "h:\\" << testDrives["H"];

    QTest::newRow("I:\\") << "I:\\" << testDrives["I"];

    QTest::newRow("L:\\") << "L:\\" << testDrives["L"];

    QTest::newRow("net share") << "//192.168.7.55/Users/" << testDrives["invalid"];
#elif defined(Q_OS_MAC)
    QTest::newRow("/") << "/" << testDrives["root"];
    QTest::newRow("/Volumes/Macintosh HD") << "/Volumes/Macintosh HD" << testDrives["root"];
    QTest::newRow("/Volumes/Data HD") << "/Volumes/Data HD" << testDrives["root"];

    QTest::newRow("/net") << "/net" << testDrives["share1"];
#elif defined(Q_OS_LINUX)
    // ###
#elif defined(Q_OS_SYMBIAN)
    // ###
#else
    // ###
#endif
}

void tst_QDriveInfo::equals_data()
{
    QTest::addColumn<QDriveInfo>("info");

    QTest::newRow("invalid") << QDriveInfo();

    foreach (const QDriveInfo &info, QDriveInfo::drives())
        QTest::newRow(info.device()) << info;
}

void tst_QDriveInfo::equals()
{
    QFETCH(QDriveInfo, info);

    QDriveInfo info2 = info;
    QDriveInfo info3(info);
    QDriveInfo info4(info.rootPath());
    QDriveInfo info5(info.device());

    QVERIFY(info2 == info);
    QVERIFY(info3 == info);
    QVERIFY(info4 == info);
#if 0
    QVERIFY(info5 != info); // ### QVERIFY(info5 == info);
#endif
}

void tst_QDriveInfo::rootDrive()
{
    QDriveInfo rootDrive = QDriveInfo::rootDrive();
    QVERIFY(rootDrive.isValid());
    QVERIFY(rootDrive.isReady());
    QVERIFY(rootDrive.isRoot());
}

void tst_QDriveInfo::constructor_data()
{
    common_data();
}

void tst_QDriveInfo::constructor()
{
    QFETCH(QString, path);
    QFETCH(DriveInfo, driveInfo);

    QDriveInfo info(path);
    QCOMPARE(info.isValid(), driveInfo.isValid);
    QCOMPARE(info.isReady(), driveInfo.isReady);
    QCOMPARE(info.isRoot(), driveInfo.isRoot);
    QCOMPARE(info.rootPath(), driveInfo.rootPath);

    QCOMPARE(QLatin1String(info.device()), QLatin1String(driveInfo.device));
    QCOMPARE(QLatin1String(info.fileSystemName()), QLatin1String(driveInfo.fileSystemName));
    QCOMPARE(info.name(), driveInfo.name);

    QCOMPARE(info.type(), driveInfo.type);
    QCOMPARE((uint)info.capabilities(), driveInfo.capabilities);

    QVERIFY(info.bytesFree() <= info.bytesTotal());
    QVERIFY(info.bytesAvailable() <= info.bytesTotal());
}

void tst_QDriveInfo::setRootPath_data()
{
    common_data();
}

void tst_QDriveInfo::setRootPath()
{
    QFETCH(QString, path);
    QFETCH(DriveInfo, driveInfo);

    QDriveInfo info;
    QVERIFY(!info.isValid());
    QVERIFY(!info.isReady());
    QVERIFY(!info.isRoot());
    QCOMPARE(info.rootPath(), QString());

    info.setRootPath(path);
    QCOMPARE(info.isValid(), driveInfo.isValid);
    QCOMPARE(info.isReady(), driveInfo.isReady);
    QCOMPARE(info.isRoot(), driveInfo.isRoot);
    QCOMPARE(info.rootPath(), driveInfo.rootPath);

    QCOMPARE(QLatin1String(info.device()), QLatin1String(driveInfo.device));
    QCOMPARE(QLatin1String(info.fileSystemName()), QLatin1String(driveInfo.fileSystemName));
    QCOMPARE(info.name(), driveInfo.name);

    QCOMPARE(info.type(), driveInfo.type);
    QCOMPARE((uint)info.capabilities(), driveInfo.capabilities);

    QVERIFY(info.bytesFree() <= info.bytesTotal());
    QVERIFY(info.bytesAvailable() <= info.bytesTotal());

    info.setRootPath("***" + path); // invalid path
    QVERIFY(!info.isValid());
    QVERIFY(!info.isReady());
    QVERIFY(!info.isRoot());
    QCOMPARE(info.rootPath(), QString());

    info.setRootPath(path); // and again
    QCOMPARE(info.isValid(), driveInfo.isValid);
    QCOMPARE(info.isReady(), driveInfo.isReady);
    QCOMPARE(info.isRoot(), driveInfo.isRoot);
    QCOMPARE(info.rootPath(), driveInfo.rootPath);
}

void tst_QDriveInfo::refresh_data()
{
    common_data();
}

void tst_QDriveInfo::refresh()
{
    QFETCH(QString, path);
    QFETCH(DriveInfo, driveInfo);

    QDriveInfo info("***" + path); // invalid path
    QVERIFY(!info.isValid());
    QVERIFY(!info.isReady());
    QVERIFY(!info.isRoot());
    QCOMPARE(info.rootPath(), QString());

    info.refresh();
    QVERIFY(!info.isValid());
    QVERIFY(!info.isReady());
    QVERIFY(!info.isRoot());
    QCOMPARE(info.rootPath(), QString());

    info.setRootPath(path);
    QCOMPARE(info.isValid(), driveInfo.isValid);
    QCOMPARE(info.isReady(), driveInfo.isReady);
    QCOMPARE(info.isRoot(), driveInfo.isRoot);
    QCOMPARE(info.rootPath(), driveInfo.rootPath);

    QCOMPARE(QLatin1String(info.device()), QLatin1String(driveInfo.device));
    QCOMPARE(QLatin1String(info.fileSystemName()), QLatin1String(driveInfo.fileSystemName));
    QCOMPARE(info.name(), driveInfo.name);

    QCOMPARE(info.type(), driveInfo.type);
    QCOMPARE((uint)info.capabilities(), driveInfo.capabilities);

    QVERIFY(info.bytesFree() <= info.bytesTotal());
    QVERIFY(info.bytesAvailable() <= info.bytesTotal());
}

void tst_QDriveInfo::drives_data()
{
    QTest::addColumn<QDriveInfo::DriveType>("type");

    QTest::newRow("local") << QDriveInfo::InternalDrive;
    QTest::newRow("cdrom") << QDriveInfo::CdromDrive;
    QTest::newRow("removable") << QDriveInfo::RemovableDrive;
    QTest::newRow("remote") << QDriveInfo::RemoteDrive;
}

void tst_QDriveInfo::drives()
{
    QFETCH(QDriveInfo::DriveType, type);

    QStringList drives;
    foreach (const DriveInfo &info, testDrives.values()) {
        if (info.type == type)
            drives << info.rootPath;
    }

    foreach (const QDriveInfo &info, QDriveInfo::drives()) {
        QVERIFY(info.isValid());
        if (info.type() == type) {
            QVERIFY(drives.removeAll(info.rootPath()) == 1);
            if (type != QDriveInfo::CdromDrive) {
                QVERIFY(info.isReady());
                QVERIFY(!(info.capabilities() & QDriveInfo::ReadOnlyVolume));
            } else {
                QCOMPARE(info.isReadOnly(), info.isReady());
            }
        }
    }
    QVERIFY(drives.isEmpty());
}

QTEST_APPLESS_MAIN(tst_QDriveInfo)

#include "tst_qdriveinfo.moc"
