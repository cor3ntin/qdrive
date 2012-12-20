/****************************************************************************
**
** Copyright (C) 2012 Ivan Komissarov
** Contact: http://www.qt-project.org/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QDRIVEINFO_H
#define QDRIVEINFO_H

#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QExplicitlySharedDataPointer>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QDriveInfoPrivate;
class Q_CORE_EXPORT QDriveInfo
{
public:
    enum DriveType {
        UnknownDrive = 0,
        InternalDrive,
        RemovableDrive,
        RemoteDrive,
        CdromDrive,
        InternalFlashDrive,
        RamDrive
    };

    QDriveInfo();
    explicit QDriveInfo(const QString &rootPath);
    QDriveInfo(const QDriveInfo &other);
    ~QDriveInfo();

    QDriveInfo &operator=(const QDriveInfo &other);

    bool operator==(const QDriveInfo &other) const;
    inline bool operator!=(const QDriveInfo &other) const;

    QString rootPath() const;
    void setRootPath(const QString &);

    QByteArray device() const;
    QByteArray fileSystemName() const;
    QString name() const;

    quint64 bytesTotal() const;
    quint64 bytesFree() const;
    quint64 bytesAvailable() const;

    inline bool isRoot() const;
    bool isReadOnly() const;
    bool isReady() const;
    bool isValid() const;

    DriveType type() const;

    void refresh();

    static QList<QDriveInfo> drives();
    static QDriveInfo rootDrive();

protected:
    QExplicitlySharedDataPointer<QDriveInfoPrivate> d_ptr;

private:
    void detach();

    inline QDriveInfoPrivate* d_func()
    {
        detach();
        return const_cast<QDriveInfoPrivate*>(d_ptr.constData());
    }

    inline const QDriveInfoPrivate* d_func() const
    {
        return d_ptr.constData();
    }
    friend class QDriveInfoPrivate;
};

inline bool QDriveInfo::operator!=(const QDriveInfo &other) const
{ return !(operator==(other)); }

inline bool QDriveInfo::isRoot() const
{ return *this == QDriveInfo::rootDrive(); }

QT_END_NAMESPACE

QT_END_HEADER

#endif // QDRIVEINFO_H
