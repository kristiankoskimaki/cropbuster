#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "pic.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QStringList images_found;

private slots:
    void on_scan_folders_clicked();
    void add_images_from(QDir &dir);

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
