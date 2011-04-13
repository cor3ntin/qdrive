#include "navigationmodel.h"
#include "navigationmodel_p.h"

#include <QDriveInfo>
#include <QDriveController>
#include <QDebug>

NavigationModelPrivate::NavigationModelPrivate(NavigationModel *qq) :
    q_ptr(qq)
{
}

NavigationModel::NavigationModel(QObject *parent) :
    QAbstractItemModel(parent),
    d_ptr(new NavigationModelPrivate(this))
{
    Q_D(NavigationModel);

    d->rootItem = new TreeItem();

    d->drivesItem = new TreeItem(d->rootItem, tr("Devices"));
    d->foldersItem = new TreeItem(d->rootItem, tr("Folders"));

    d->driveController = new QDriveController(this);
    connect(d->driveController, SIGNAL(driveMounted(QString)), d, SLOT(onDriveAdded(QString)));
    connect(d->driveController, SIGNAL(driveUnmounted(QString)), d, SLOT(onDriveRemoved(QString)));

    QList<QDriveInfo> drives = QDriveInfo::drives();
    foreach (const QDriveInfo &info, drives) {
        QString name = info.name();
        QString path = info.rootPath();

        qDebug() << name << path;

        TreeItem *item = new TreeItem(d->drivesItem, name, path);
        d->mapToItem.insert(path, item);
    }
}

NavigationModel::~NavigationModel()
{
    Q_D(NavigationModel);

    delete d->rootItem;

    delete d_ptr;
}

int NavigationModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 1;
}

QVariant NavigationModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    if (role == Qt::DisplayRole) {
        return item->name;
    } else if (role == Qt::UserRole) {
        if (item->type == TreeItem::ChildItem)
            return item->path;
        else
            return QVariant();
    }
    return QVariant();
}

Qt::ItemFlags NavigationModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex NavigationModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_D(const NavigationModel);

    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem *parentItem;

    if (!parent.isValid())
        parentItem = d->rootItem;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex NavigationModel::parent(const QModelIndex &index) const
{
    Q_D(const NavigationModel);

    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem *parentItem = childItem->parent();

    if (parentItem == d->rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int NavigationModel::rowCount(const QModelIndex &parent) const
{
    Q_D(const NavigationModel);

    TreeItem *parentItem;

    if (!parent.isValid())
        parentItem = d->rootItem;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

void NavigationModelPrivate::onDriveAdded(const QString &path)
{
    Q_Q(NavigationModel);

    QDriveInfo info(path);
    QString name = info.name();

    qDebug() << "onDriveAdded" << name << path;

    QModelIndex parent = q->createIndex(drivesItem->row(), 0, drivesItem);
    q->beginInsertRows(parent, drivesItem->childCount(), drivesItem->childCount());
    TreeItem *item = new TreeItem(drivesItem, name, path);
    mapToItem.insert(path, item);
    q->endInsertRows();
}

void NavigationModelPrivate::onDriveRemoved(const QString &path)
{
    Q_Q(NavigationModel);

    qDebug() << "onDriveRemoved" << path;
    TreeItem *item = mapToItem.value(path);
    if (!item)
        return;

    QModelIndex parent = q->createIndex(drivesItem->row(), 0, drivesItem);
    q->beginRemoveRows(parent, item->row(), item->row());
    delete item;
    mapToItem.remove(path);
    q->endRemoveRows();
}

QString NavigationModel::path(const QModelIndex &index) const
{
    if (!index.isValid())
        return "";

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    if (item->type == TreeItem::GroupItem)
        return "";
    else
        return item->path;
}

void NavigationModel::addFolder(const QString &/*path*/)
{
}
