#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetItem>
#include <QDirIterator>
#include <QThreadPool>
#include <QTimer>
#include <QPainter>
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

private slots:
    void closeEvent(QCloseEvent *event) { Q_UNUSED (event) stop_scanning = true; }
    void on_folders_box_returnPressed() { on_scan_folders_clicked(); }
    void on_scan_folders_clicked();
    void search_for_images(const QStringList &folders, const QString &not_found);
    void add_image_with_borders(Pic *add_me) { images_with_borders << add_me; };
    void on_images_table_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous);
    void resizeEvent(QResizeEvent *event);

private:
    Ui::MainWindow *ui;
};


//using QTimer(), an instance of this class can execute add_rows() at regular intervals to populate filename table
//event loop would be filled (=program frozen) if a new row was added every time an image with borders is found
class ImageTable : public QMainWindow {
public:
    ImageTable(QTableWidget *table_ptr, QVector<Pic *>*filenames_ptr) : table(table_ptr), filenames(filenames_ptr) { };
    QTableWidget *table;
    QVector<Pic *> *filenames;
    void add_rows();
};


#endif // MAINWINDOW_H
