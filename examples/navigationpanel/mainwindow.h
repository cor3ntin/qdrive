#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QModelIndex;
class QFileSystemModel;
class QTreeView;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void onClick(const QString & name);
    void onDoubleClick(const QModelIndex & index);

private:
    QFileSystemModel *model;
    QTreeView *view;
};

#endif // MAINWINDOW_H
