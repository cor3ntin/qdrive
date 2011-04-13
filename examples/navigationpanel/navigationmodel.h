#ifndef NAVIGATIONMODEL_H
#define NAVIGATIONMODEL_H

#include <QAbstractItemModel>

class NavigationModelPrivate;
class NavigationModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(NavigationModel)
public:
    explicit NavigationModel(QObject *parent = 0);
    ~NavigationModel();

    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;

    QString path(const QModelIndex &index) const;

    void addFolder(const QString &path);

signals:

public slots:

protected:
    NavigationModelPrivate *d_ptr;
};

#endif // NAVIGATIONMODEL_H
