#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class QModelIndex;
class QFileSystemModel;
class QTreeView;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);

private slots:
    void onClick(const QString & name);
    void onDoubleClick(const QModelIndex & index);

private:
    Ui::MainWindow *ui;
    QFileSystemModel * model;
    QTreeView * view;
};

#endif // MAINWINDOW_H
