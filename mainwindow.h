#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDragEnterEvent>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QPainter>
#include <QFileDialog>
#include <QtConcurrent/QtConcurrent>
#include <QProgressBar>
#include <QScrollBar>
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
    bool show_scaled_image = true;
    QImage image;
    int image_height, image_width;
    double vertical_scrollbar_pos = 0.0;
    double horizontal_scrollbar_pos = 0.0;

    static constexpr bool ONLY_BLACK_BORDER = true;
    static constexpr bool ANY_BORDER = false;
    bool border_preference = ONLY_BLACK_BORDER;

    static constexpr int DEACTIVATE_WIDGETS = 0;
    static constexpr int ACTIVATE_WIDGETS = 1;
    static constexpr int UNPAUSE_WIDGETS = 2;
    static constexpr int PAUSE_WIDGETS = 3;

private slots:
    void closeEvent(QCloseEvent *event) { Q_UNUSED (event) stop_scanning = true; }
    void dragEnterEvent(QDragEnterEvent *event) { if(event->mimeData()->hasUrls()) event->acceptProposedAction(); }
    void dropEvent(QDropEvent *event);
    void set_gui_state(const int &state);
    void on_browse_folders_clicked();
    void on_folders_box_returnPressed() { on_scan_folders_clicked(); }
    void on_scan_folders_clicked();
    void search_for_images(const QStringList &folders, const QString &not_found);
    void get_progressbar_max(const QStringList &folders);
    void set_progressbar_max(const int &max) { QProgressBar *bar = findChild<QProgressBar*>("progress_bar"); if (bar) bar->setMaximum(max); };
    void add_image_with_borders(Pic *add_me) { images_with_borders << add_me; };
    void on_images_table_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous);
    void draw_border_rectangle();
    void resizeEvent(QResizeEvent *event) { draw_border_rectangle(); Q_UNUSED(event) };
    void add_rows();

    void on_border_color_pref_clicked();
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
    void on_zoom_clicked() { show_scaled_image = show_scaled_image == true? false : true; draw_border_rectangle(); };

signals:
    void show_progressbar(int maxvalue);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
