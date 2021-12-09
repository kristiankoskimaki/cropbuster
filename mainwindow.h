#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetItem>
#include <QDirIterator>
#include <QThreadPool>
#include <QTimer>
#include <QPainter>
#include <QFileDialog>
#include <QProcess>
#include <QtConcurrent/QtConcurrent>
#include "pic.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() { };

    QVector<Pic *> images_with_borders;
    bool stop_scanning = false;
    QImage image;
    int image_height, image_width;

private slots:
    void closeEvent(QCloseEvent *event) { Q_UNUSED (event) stop_scanning = true; }
    void on_folders_box_returnPressed() { on_scan_folders_clicked(); }
    void on_scan_folders_clicked();
    void search_for_images(const QStringList &folders, const QString &not_found);
    void get_progressbar_max(const QStringList &folders);
    void set_progressbar_max(const int &max);
    void add_image_with_borders(Pic *add_me) { images_with_borders << add_me; };
    void on_images_table_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous);
    void draw_border_rectangle();
    void resizeEvent(QResizeEvent *event) { if(!images_with_borders.empty()) draw_border_rectangle(); Q_UNUSED(event) };
    void add_rows();

    void on_open_in_explorer_clicked();
    void on_save_as_clicked();
    void select_next_row(const int &current_row);
    void on_grow_top_clicked();
    void on_grow_bottom_clicked();
    void on_grow_left_clicked();
    void on_grow_right_clicked();
    void on_shrink_top_clicked();
    void on_shrink_bottom_clicked();
    void on_shrink_left_clicked();
    void on_shrink_right_clicked();

signals:
    void show_progressbar(int maxvalue);

private:
    Ui::MainWindow *ui;
};
/*
class ClickableLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ClickableLabel(QWidget *parent) { Q_UNUSED (parent) }
protected:
    void mousePressEvent(QMouseEvent *event) { Q_UNUSED (event) emit clicked(); }
signals:
    void clicked();
};
*/
#endif // MAINWINDOW_H
