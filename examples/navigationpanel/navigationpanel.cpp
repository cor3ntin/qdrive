#include "navigationpanel.h"

#include "navigationmodel.h"

#include <QtCore/QDir>
#include <QtGui/QTreeWidget>
#include <QtGui/QResizeEvent>
#include <QtGui/QDesktopServices>
#include <QDebug>

QFileInfoList drives()
{
#ifndef Q_OS_MAC
    return QDir::drives());
#endif
#ifdef Q_OS_MAC
    return QDir("/Volumes").entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot);
#endif
}

NavigationPanel::NavigationPanel(QWidget *parent) :
    QWidget(parent),
    m_treeView(new QTreeView(this))
{
    setMinimumSize(200, 200);
    m_treeView->setHeaderHidden(true);

    m_model = new NavigationModel(this);
    m_treeView->setModel(m_model);
    m_treeView->setFocusPolicy(Qt::NoFocus);

    QPalette pal = m_treeView->palette();
    pal.setColor(QPalette::Base, QColor(214, 221, 229));
    m_treeView->QAbstractItemView::setPalette(pal);
    m_treeView->expandAll();

    connect(m_treeView, SIGNAL(clicked(QModelIndex)), SLOT(onClick(QModelIndex)));
}

NavigationPanel::~NavigationPanel()
{
}

void NavigationPanel::addFolder(const QString & path)
{
    m_model->addFolder(path);
}

void NavigationPanel::resizeEvent(QResizeEvent * event)
{
    m_treeView->resize(event->size());
    QWidget::resizeEvent(event);
}

void NavigationPanel::onClick(QTreeWidgetItem *item, int column)
{
    if (!m_groups.contains(item->text(0)))
        emit folderClicked(item->data(0, Qt::UserRole).toString());
}

void NavigationPanel::onClick(const QModelIndex &index)
{
    QString path = m_model->path(index);
    if (!path.isEmpty())
        emit folderClicked(path);
}


