#ifndef QNAVIGATIONPANEL_H
#define QNAVIGATIONPANEL_H

#include <QWidget>
#include <QHash>

class NavigationModel;
class QModelIndex;
class QTreeView;
class QTreeWidget;
class QTreeWidgetItem;
class NavigationPanel : public QWidget
{
    Q_OBJECT
public:
    explicit NavigationPanel(QWidget *parent = 0);
    ~NavigationPanel();
    void addFolder(const QString & path);

protected:
    virtual void resizeEvent(QResizeEvent *);

signals:
    void folderClicked(const QString & name);

private slots:
    void onClick(QTreeWidgetItem*,int);
    void onClick(const QModelIndex &index);

private:
    QTreeView *m_treeView;
    NavigationModel *m_model;
    QHash<QString, QTreeWidgetItem *> m_groups;
//    QHash<QString, QTreeWidgetItem *> m_folders;
};

#endif // QNAVIGATIONPANEL_H
