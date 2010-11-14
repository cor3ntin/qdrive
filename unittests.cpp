#include <QtTest>

#include "qdriveinfo.h"

Q_DECLARE_METATYPE(QDriveInfo::DriveType)
Q_DECLARE_METATYPE(QDriveInfo::Capabilities)

class tst_QDriveInfo: public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void constructor_data();
    void constructor();
    void setRootPath_data();
    void setRootPath();
    void refresh_data();
    void refresh();
    void drives_data();
    void drives();
};

void tst_QDriveInfo::initTestCase()
{
    qRegisterMetaType<QDriveInfo::DriveType>();
    qRegisterMetaType<QDriveInfo::Capabilities>();
}

void tst_QDriveInfo::constructor_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<bool>("isValid");
    QTest::addColumn<bool>("isReady");
    QTest::addColumn<QString>("rootPath");
    QTest::addColumn<QString>("device");
    QTest::addColumn<QString>("fileSystemName");
    QTest::addColumn<QString>("name");
    QTest::addColumn<QDriveInfo::DriveType>("type");
    QTest::addColumn<QDriveInfo::Capabilities>("capabilities");

    QTest::newRow("invalid")
        << "" << false << false << "" << "" << "" << "" << QDriveInfo::InvalidDrive << QDriveInfo::Capabilities(0);

#if defined(Q_OS_WIN)
    QString guid_for_d("\\\\?\\Volume{e9204280-a0d1-11df-93bf-806d6172696f}\\");
    QString guid_for_i("\\\\?\\Volume{2c15bbb2-bf3c-11df-8730-0004619ece73}\\");
    QDriveInfo::Capabilities caps_for_ntfs = (QDriveInfo::AccessControlListsSupport | QDriveInfo::HardlinksSupport);

    QTest::newRow("invalid_P:")
        << "P:" << false << false << "P:/" << "" << "" << "" << QDriveInfo::InvalidDrive << QDriveInfo::Capabilities(0);
    QTest::newRow("invalid_PP:/")
        << "PP:/" << false << false << "" << "" << "" << "" << QDriveInfo::InvalidDrive << QDriveInfo::Capabilities(0);

    QTest::newRow("d:\\")
        << "d:\\" << true << true << "d:/" << guid_for_d << "NTFS" << "old win" << QDriveInfo::InternalDrive << caps_for_ntfs;
    QTest::newRow("D:\\windows\\")
        << "D:\\windows\\" << true << true << "D:/" << guid_for_d << "NTFS" << "old win" << QDriveInfo::InternalDrive << caps_for_ntfs;
    QTest::newRow("D:\\windows\\..")
        << "D:\\windows\\.." << true << true << "D:/" << guid_for_d << "NTFS" << "old win" << QDriveInfo::InternalDrive << caps_for_ntfs;

    QTest::newRow("I:\\")
        << "I:\\" << true << false << "I:/" << guid_for_i << "" << "" << QDriveInfo::CdromDrive << QDriveInfo::Capabilities(0);

    QTest::newRow("net share")
        << "//192.168.7.55/Users/" << true << true << "//192.168.7.55/Users/" << "" << "NTFS" << "" << QDriveInfo::RemoteDrive << caps_for_ntfs;
#elif defined(Q_OS_MAC)
    QDriveInfo::Capabilities caps_for_hfs = (QDriveInfo::AccessControlListsSupport | QDriveInfo::HardlinksSupport);

    QTest::newRow("/")
        << "/Volumes/Macintosh HD" << true << true << "/" << "/dev/disk0s2" << "hfs" << "Macintosh HD" << QDriveInfo::InvalidDrive << caps_for_hfs;
    QTest::newRow("/Volumes/Macintosh HD")
        << "/Volumes/Macintosh HD" << true << true << "/" << "/dev/disk0s2" << "hfs" << "Macintosh HD" << QDriveInfo::InvalidDrive << caps_for_hfs;
    QTest::newRow("/Volumes/Data HD")
        << "/Volumes/Data HD" << true << true << "/" << "/dev/disk0s2" << "hfs" << "Macintosh HD" << QDriveInfo::InvalidDrive << caps_for_hfs;

    // ###
    QTest::newRow("/net")
        << "/net" << true << true << "" << "" << "" << "" << QDriveInfo::RemoteDrive << QDriveInfo::Capabilities(0);
#elif defined(Q_OS_LINUX)
    // ###
#endif
}

void tst_QDriveInfo::constructor()
{
    QFETCH(QString, path);
    QFETCH(bool, isValid);
    QFETCH(bool, isReady);
    QFETCH(QString, rootPath);
    QFETCH(QString, device);
    QFETCH(QString, fileSystemName);
    QFETCH(QString, name);
    QFETCH(QDriveInfo::DriveType, type);
    QFETCH(QDriveInfo::Capabilities, capabilities);

    QDriveInfo info(path);
    QCOMPARE(info.isValid(), isValid);
    QCOMPARE(info.isReady(), isReady);
    QCOMPARE(info.rootPath(), rootPath);

    QCOMPARE(QString(info.device()), device);
    QCOMPARE(QString(info.fileSystemName()), fileSystemName);
    QCOMPARE(info.name(), name);

    QVERIFY(info.bytesFree() <= info.bytesTotal());
    QVERIFY(info.bytesAvailable() <= info.bytesTotal());
    QVERIFY(info.bytesAvailable() <= info.bytesFree());

    QCOMPARE(info.type(), type);
    QCOMPARE(info.capabilities(), capabilities);
}

void tst_QDriveInfo::setRootPath_data()
{
    // same test
    constructor_data();
}

void tst_QDriveInfo::setRootPath()
{
    QFETCH(QString, path);
    QFETCH(bool, isValid);
    QFETCH(bool, isReady);
    QFETCH(QString, rootPath);
    QFETCH(QString, device);
    QFETCH(QString, fileSystemName);
    QFETCH(QString, name);
    QFETCH(QDriveInfo::DriveType, type);
    QFETCH(QDriveInfo::Capabilities, capabilities);

    QDriveInfo info;
    QVERIFY(!info.isValid());
    QVERIFY(!info.isReady());
    QCOMPARE(info.rootPath(), QString());

    info.setRootPath(path);
    QCOMPARE(info.isValid(), isValid);
    QCOMPARE(info.isReady(), isReady);
    QCOMPARE(info.rootPath(), rootPath);

    QCOMPARE(QString(info.device()), device);
    QCOMPARE(QString(info.fileSystemName()), fileSystemName);
    QCOMPARE(info.name(), name);

    QVERIFY(info.bytesFree() <= info.bytesTotal());
    QVERIFY(info.bytesAvailable() <= info.bytesTotal());
    QVERIFY(info.bytesAvailable() <= info.bytesFree());

    QCOMPARE(info.type(), type);
    QCOMPARE(info.capabilities(), capabilities);

    info.setRootPath("***" + path);

    QVERIFY(!info.isValid());
    QVERIFY(!info.isReady());
    QCOMPARE(info.rootPath(), QString());

    info.setRootPath(path);

    QCOMPARE(info.isValid(), isValid);
    QCOMPARE(info.isReady(), isReady);
    QCOMPARE(info.rootPath(), rootPath);
}

void tst_QDriveInfo::refresh_data()
{
    // same test
    constructor_data();
}

void tst_QDriveInfo::refresh()
{
    QFETCH(QString, path);
    QFETCH(bool, isValid);
    QFETCH(bool, isReady);
    QFETCH(QString, rootPath);
    QFETCH(QString, device);
    QFETCH(QString, fileSystemName);
    QFETCH(QString, name);
    QFETCH(QDriveInfo::DriveType, type);
    QFETCH(QDriveInfo::Capabilities, capabilities);

    QDriveInfo info("***" + path);
    QVERIFY(!info.isValid());
    QVERIFY(!info.isReady());
    QCOMPARE(info.rootPath(), QString());

    info.refresh();
    QVERIFY(!info.isValid());
    QVERIFY(!info.isReady());
    QCOMPARE(info.rootPath(), QString());

    info.setRootPath(path);

    QCOMPARE(info.isValid(), isValid);
    QCOMPARE(info.isReady(), isReady);
    QCOMPARE(info.rootPath(), rootPath);

    QCOMPARE(QString(info.device()), device);
    QCOMPARE(QString(info.fileSystemName()), fileSystemName);
    QCOMPARE(info.name(), name);

    QVERIFY(info.bytesFree() <= info.bytesTotal());
    QVERIFY(info.bytesAvailable() <= info.bytesTotal());
    QVERIFY(info.bytesAvailable() <= info.bytesFree());

    QCOMPARE(info.type(), type);
    QCOMPARE(info.capabilities(), capabilities);
}

void tst_QDriveInfo::drives_data()
{
    QTest::addColumn<QStringList>("drives");
    QTest::addColumn<QDriveInfo::DriveType>("type");

    QStringList localDrives;
    QStringList cdromDrives;
    QStringList removableDrives;
    QStringList remoteDrives;

#if defined(Q_OS_WIN)
    localDrives << "C:/" << "D:/" << "E:/" << "F:/" << "H:/";
    cdromDrives << "I:/";
    removableDrives << "L:/";
#elif defined(Q_OS_MAC)
    localDrives << "/" << "/Volumes/Data HD" << "/home";
    remoteDrives << "/net";
#elif defined(Q_OS_LINUX)

#endif

    QTest::newRow("local") << localDrives << QDriveInfo::InternalDrive;
    QTest::newRow("cdrom") << cdromDrives << QDriveInfo::CdromDrive;
    QTest::newRow("removable") << removableDrives << QDriveInfo::RemovableDrive;
    QTest::newRow("remote") << remoteDrives << QDriveInfo::RemoteDrive;
}

void tst_QDriveInfo::drives()
{
    QFETCH(QStringList, drives);
    QFETCH(QDriveInfo::DriveType, type);

    foreach (const QDriveInfo &info, QDriveInfo::drives()) {
        QVERIFY(info.isValid());
        if (info.type() == type) {
            QVERIFY(drives.removeAll(info.rootPath()) == 1);
            if (type != QDriveInfo::CdromDrive) {
                QVERIFY(info.isReady());
                QVERIFY(!(info.capabilities() & QDriveInfo::ReadOnlyVolume));
            } else {
                QVERIFY((info.capabilities() & QDriveInfo::ReadOnlyVolume) == info.isReady());
            }
        }
    }
    QVERIFY(drives.isEmpty());
}

QTEST_APPLESS_MAIN(tst_QDriveInfo)

#include "unittests.moc"
