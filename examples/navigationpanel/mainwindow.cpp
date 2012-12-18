#include "mainwindow.h"

#include <QDockWidget>
#include <QTreeView>
#include <QFileSystemModel>
#include <QStandardPaths>

#include "navigationpanel.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    QDockWidget * dock = new QDockWidget();

    NavigationPanel * panel = new NavigationPanel(dock);
    dock->setWidget(panel);
    dock->setTitleBarWidget(new QWidget); // remove TitleBar

    addDockWidget(Qt::LeftDockWidgetArea, dock);

    model = new QFileSystemModel(this);
    model->setRootPath("/");
    model->setFilter(QDir::AllDirs | QDir::System | QDir::Hidden | QDir::Files /*| QDir::NoDotAndDotDot*/);

    view = new QTreeView(this);
    view->setModel(model);
    view->setItemsExpandable(false);
    view->setRootIsDecorated(false);
    view->setRootIndex(model->index(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)));

    setCentralWidget(view);

    connect(panel, SIGNAL(folderClicked(QString)), this, SLOT(onClick(QString)));
    connect(view, SIGNAL(doubleClicked(QModelIndex)), SLOT(onDoubleClick(QModelIndex)));

    resize(640, 480);
}

MainWindow::~MainWindow()
{
}

void MainWindow::onClick(const QString & path)
{
    view->setRootIndex(model->index(path));
}

void MainWindow::onDoubleClick(const QModelIndex &index)
{
    QString path = model->filePath(index);
    path = QFileInfo(path).canonicalFilePath();
    view->setRootIndex(model->index(path));
}
