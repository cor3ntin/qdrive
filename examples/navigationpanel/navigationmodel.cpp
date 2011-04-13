#include "navigationmodel.h"
#include "navigationmodel_p.h"

#include <QDriveInfo>
#include <QDriveController>
#include <QDesktopServices>
#include <QFileInfo>
#include <QDebug>

NavigationModelPrivate::NavigationModelPrivate(NavigationModel *qq) :
    q_ptr(qq)
{
}

void NavigationModelPrivate::insertItem(TreeItem *parentItem, const QString &name, const QString &path)
{
    Q_Q(NavigationModel);

    QModelIndex parent = q->createIndex(parentItem->row(), 0, parentItem);
    q->beginInsertRows(parent, parentItem->childCount(), parentItem->childCount());
    TreeItem *item = new TreeItem(parentItem, name, path);
    mapToItem.insert(path, item);
    q->endInsertRows();
}

void NavigationModelPrivate::removeItem(TreeItem *parentItem, const QString &path)
{
    Q_Q(NavigationModel);

    TreeItem *item = mapToItem.value(path);
    if (!item)
        return;

    QModelIndex parent = q->createIndex(parentItem->row(), 0, parentItem);
    q->beginRemoveRows(parent, item->row(), item->row());
    delete item;
    mapToItem.remove(path);
    q->endRemoveRows();
}

void NavigationModelPrivate::onDriveAdded(const QString &path)
{
    QDriveInfo info(path);
    QString name = info.name();

    qDebug() << "onDriveAdded" << name << path;

    if (info.type() == QDriveInfo::RemoteDrive)
        insertItem(networkItem, name, path);
    else
        insertItem(drivesItem, name, path);
}

void NavigationModelPrivate::onDriveRemoved(const QString &path)
{
    qDebug() << "onDriveRemoved" << path;

    removeItem(drivesItem, path);
}

NavigationModel::NavigationModel(QObject *parent) :
    QAbstractItemModel(parent),
    d_ptr(new NavigationModelPrivate(this))
{
    Q_D(NavigationModel);

    d->rootItem = new TreeItem();

    d->drivesItem = new TreeItem(d->rootItem, tr("Devices"));
    d->networkItem = new TreeItem(d->rootItem, tr("Network"));
    d->foldersItem = new TreeItem(d->rootItem, tr("Folders"));

    d->driveController = new QDriveController(this);
    connect(d->driveController, SIGNAL(driveMounted(QString)), d, SLOT(onDriveAdded(QString)));
    connect(d->driveController, SIGNAL(driveUnmounted(QString)), d, SLOT(onDriveRemoved(QString)));

    QList<QDriveInfo> drives = QDriveInfo::drives();
    foreach (const QDriveInfo &info, drives) {
        QString name = info.name();
        QString path = info.rootPath();

        qDebug() << name << path << info.type();

        TreeItem *item = 0;
        if (info.type() == QDriveInfo::RemoteDrive)
            item = new TreeItem(d->networkItem, name, path);
        else
            item = new TreeItem(d->drivesItem, name, path);

        d->mapToItem.insert(path, item);
    }

    StandardLocations locations(DesktopLocation | DocumentsLocation | HomeLocation | ApplicationsLocation);
    setStandardLocations(locations);
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

void NavigationModel::addFolder(const QString &path)
{
    Q_D(NavigationModel);

    QFileInfo info(path);
    QString name = info.fileName();

    if (d->mapToItem.contains(info.canonicalFilePath()))
        return;

    qDebug() << "addFolder" << name << path;

    d->insertItem(d->foldersItem, name, info.canonicalFilePath());
}

void NavigationModel::removeFolder(const QString &path)
{
    Q_D(NavigationModel);

    d->removeItem(d->foldersItem, path);
}

NavigationModel::StandardLocations NavigationModel::standardLocations() const
{
    Q_D(const NavigationModel);

    return d->locations;
}

void NavigationModel::setStandardLocations(StandardLocations locations)
{
    Q_D(NavigationModel);

    d->locations = locations;

    QString path;

    path = QDesktopServices::storageLocation(QDesktopServices::DesktopLocation);
    if (locations & DesktopLocation)
        addFolder(path);
    else
        removeFolder(path);

    path = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
    if (locations & HomeLocation)
        addFolder(path);
    else
        removeFolder(path);

    path = QDesktopServices::storageLocation(QDesktopServices::ApplicationsLocation);
    if (locations & ApplicationsLocation)
        addFolder(path);
    else
        removeFolder(path);

    path = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
    if (locations & DocumentsLocation)
        addFolder(path);
    else
        removeFolder(path);

    path = QDesktopServices::storageLocation(QDesktopServices::MusicLocation);
    if (locations & MusicLocation)
        addFolder(path);
    else
        removeFolder(path);

    path = QDesktopServices::storageLocation(QDesktopServices::MoviesLocation);
    if (locations & MoviesLocation)
        addFolder(path);
    else
        removeFolder(path);

    path = QDesktopServices::storageLocation(QDesktopServices::PicturesLocation);
    if (locations & PicturesLocation)
        addFolder(path);
    else
        removeFolder(path);

}
