#include "qdrivecontroller_p.h"

#include <QtCore/QCoreApplication>

#ifndef SYMBIAN_3_1
#include <driveinfo.h>
#endif //SYMBIAN_3_1

Watcher *Watcher::watcher = 0;

Watcher::Watcher(QObject *parent) :
        QObject(parent),
        CActive(EPriorityStandard)
{
    CActiveScheduler::Add(this);
    if (iFs.Connect() == KErrNone) {
#ifndef SYMBIAN_3_1
        m_previousDriveList.Copy(PopulateDriveList());
#endif //SYMBIAN_3_1
        startMonitoring();
    }
}

Watcher::~Watcher()
{
    Cancel();
    iFs.Close();
}

void Watcher::DoCancel()
{
    iFs.NotifyChangeCancel();
}

void Watcher::RunL()
{
#ifdef SYMBIAN_3_1
    TDriveInfo driveInfo;
    TDriveNumber driveLetter = EDriveE;  // Have to use hardcoded MMC drive letter for 3.1
    if (iFs.Drive(driveInfo, driveLetter) == KErrNone) {
        bool driveInserted = false;

        switch (driveInfo.iType) {
        case EMediaNotPresent:
            driveInserted = false;
            break;
        default:
            driveInserted = true;
            break;
        }

        TChar volumeChar;
        QString volume;
        if (RFs::DriveToChar(driveLetter, volumeChar) == KErrNone)
            volume = QChar(volumeChar).toAscii();

        foreach (MStorageStatusObserver *observer, m_observers)
            observer->storageStatusChanged(driveInserted, volume);
    }
#else // SYMBIAN_3_1
    CompareDriveLists(PopulateDriveList());
#endif // SYMBIAN_3_1
    startMonitoring();
}

#ifndef SYMBIAN_3_1
TDriveList Watcher::PopulateDriveList()
{
    TDriveList driveList;
    TInt driveCount = 0;
    if (DriveInfo::GetUserVisibleDrives(iFs, driveList, driveCount,
            KDriveAttExclude | KDriveAttRemote | KDriveAttRom | KDriveAttInternal) != KErrNone) {
        return TDriveList();
    }

    for (int i = 0; i < KMaxDrives; i++) {
        if (driveList[i] == KDriveAbsent)
            continue;

        TUint driveStatus;
        if (DriveInfo::GetDriveStatus(iFs, i, driveStatus) == KErrNone) {

            if (!(driveStatus & DriveInfo::EDriveRemovable)) {
                driveList[i] = KDriveAbsent;
                continue;
            }

            TDriveInfo driveInfo;
            if (iFs.Drive(driveInfo, i) == KErrNone) {
                if (driveInfo.iType == EMediaNotPresent) {
                    driveList[i] = KDriveAbsent;
                }
            }
        }
    }
    return driveList;
}

void Watcher::CompareDriveLists(const TDriveList &aDriveList)
{
    if (!(aDriveList.Length() > KMaxDrives - 1) || !(m_previousDriveList.Length() > KMaxDrives - 1))
        return;

    for (int i = 0; i < KMaxDrives; i++) {

        if (aDriveList[i] == KDriveAbsent && m_previousDriveList[i] == KDriveAbsent) {
            continue;
        } else if (aDriveList[i] > KDriveAbsent && m_previousDriveList[i] > KDriveAbsent) {
            continue;
        } else if (aDriveList[i] == KDriveAbsent && m_previousDriveList[i] > KDriveAbsent) {
            TChar volumeChar;
            QString volume;
//            bool driveInserted = false;
            if (iFs.DriveToChar(i, volumeChar) == KErrNone)
                volume = QChar(volumeChar).toAscii();

            emit driveRemoved(volume);

            break;
        } else if (aDriveList[i] > KDriveAbsent && m_previousDriveList[i] == KDriveAbsent) {
            TChar volumeChar;
            QString volume;
//            bool driveInserted = true;
            if (iFs.DriveToChar(i, volumeChar) == KErrNone)
                volume = QChar(volumeChar).toAscii();

            emit driveAdded(volume);

            break;
        }
    }
    m_previousDriveList.Copy(aDriveList);
}
#endif //SYMBIAN_3_1

void Watcher::startMonitoring()
{
    iFs.NotifyChange(ENotifyDisk, iStatus);
    SetActive();
}

QDriveControllerPrivate::QDriveControllerPrivate()
{
    if (!Watcher::watcher) {
        // TODO: add mutex
        Watcher::watcher = new Watcher(qApp);
    }
}

QDriveControllerPrivate::~QDriveControllerPrivate()
{

}
