#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetItem>
#include <QThreadPool>
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
    QVector<Pic *> images_with_borders;

private slots:
    void on_folders_box_returnPressed();
    void on_scan_folders_clicked();
    void add_images_from(QDir &dir, QThreadPool &thread_pool);
    void add_image_with_borders(Pic *add_me);
    void on_images_table_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous);

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
