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
    QVector<Pic *> images_with_borders;

private slots:
    void on_scan_folders_clicked();
    void add_images_from(QDir &dir);
    void find_images_with_borders();
    void add_image_with_borders(Pic *add_me);

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
