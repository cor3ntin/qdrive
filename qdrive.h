#ifndef QDRIVE_H
#define QDRIVE_H

#include <QtCore/QString>
#include <QtCore/QStringList>

class QDrivePrivate;
class QDrive : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QDrive);

public:
    QDrive(const QString &rootPath);
    ~QDrive();

    static QStringList drivePaths();

    quint64 availableSize();
    QString fileSystemName();
    quint64 freeSize();
    QString device() const;
    QString name();
    bool    ready();
    QString rootPath();
    quint64 size();

protected:
    QDrivePrivate *d_ptr;
    void timerEvent(QTimerEvent *);
};

#endif // QDRIVE_H
