#include <QThreadPool>
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_scan_folders_clicked() {
    const QStringList folders = ui->folders_box->text().remove(QStringLiteral("\"")).split(QStringLiteral(";"));
    images_found.clear();   //new search, clear old results

    for(auto &directory : folders) {
        QDir dir = directory;
        if(dir.isEmpty())
            continue;
        if(dir.exists())
            add_images_from(dir);
    }

    find_images_with_borders();
}

void MainWindow::add_images_from(QDir &dir) {
    dir.setNameFilters(QStringList( { "*.jpg", "*.webp", "*.png" } ));
    QDirIterator iter(dir, QDirIterator::Subdirectories);
    while(iter.hasNext()) {
        const QFile file(iter.next());
        const QString filename = file.fileName();

        bool duplicate = false;                 //don't add same file many times
        for(const auto &alreadyAddedFile : images_found)
            if(filename.toLower() == alreadyAddedFile.toLower()) {
                duplicate = true;
                break;
            }
        if(!duplicate)
            images_found << filename;
    }
}

void MainWindow::find_images_with_borders() {
    if(images_found.isEmpty())
        return;

    QThreadPool thread_pool;
    for(const auto &filename : images_found) {
        auto *picture = new Pic(this, filename);
        picture->setAutoDelete(false);
        thread_pool.start(picture);

        while(thread_pool.activeThreadCount() == thread_pool.maxThreadCount())
            QApplication::processEvents();          //avoid blocking signals in event loop
    }
    thread_pool.waitForDone();
        QApplication::processEvents();              //process signals from last threads
}

void MainWindow::add_image_with_borders(Pic *add_me) {
    images_with_borders << add_me;
}
