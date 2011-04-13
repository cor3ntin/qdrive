#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDockWidget>
#include <QTreeView>
#include <QFileSystemModel>
#include <QVBoxLayout>
#include <QDebug>

#include "navigationpanel.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QDockWidget * dock = new QDockWidget();

    NavigationPanel * panel = new NavigationPanel(dock);
    dock->setWidget(panel);

    addDockWidget(Qt::LeftDockWidgetArea, dock);
    dock->setTitleBarWidget(new QWidget);
    model = new QFileSystemModel(this);
    model->setRootPath("/");
    model->setFilter(QDir::AllDirs | QDir::System | QDir::Hidden /*| QDir::NoDotAndDotDot*/);
    view = new QTreeView();
    view->setModel(model);
    view->setItemsExpandable(false);
    this->setCentralWidget(view);
    view->setRootIsDecorated(false);
    connect(panel, SIGNAL(folderClicked(QString)), this, SLOT(onClick(QString)));
    connect(view, SIGNAL(doubleClicked(QModelIndex)), SLOT(onDoubleClick(QModelIndex)));

    resize(640, 480);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::onClick(const QString & path)
{
    qDebug() << "MainWindow::onClick" << path;
    view->setRootIndex(model->index(path));
}

void MainWindow::onDoubleClick(const QModelIndex &index)
{
    QString path = model->filePath(index);
    path = QFileInfo(path).canonicalFilePath();
    qDebug() << "onDoubleClick" << path;
    view->setRootIndex(model->index(path));
}
