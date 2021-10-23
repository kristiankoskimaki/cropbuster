#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    ui->folders_box->setFocus();
    QTableWidget *table = ui->images_table;
    table->insertColumn(0);
    table->setHorizontalHeaderLabels( QStringList( "Images" ));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_folders_box_returnPressed() {
    ui->scan_folders->click();
}

void MainWindow::on_scan_folders_clicked() {
    const QStringList folders = ui->folders_box->text().remove(QStringLiteral("\"")).split(QStringLiteral(";"));
    QThreadPool pool;

    for(auto &directory : folders) {
        QDir dir = directory;
        if(dir.isEmpty())
            continue;
        if(dir.exists())
            add_images_from(dir, pool);
    }
    pool.waitForDone();
    QApplication::processEvents();              //process signals from last threads
    ui->statusbar->clearMessage();
}

void MainWindow::add_images_from(QDir &dir, QThreadPool &thread_pool) {
    dir.setNameFilters(QStringList( { "*.jpg", "*.jpeg" } ));
    QDirIterator iter(dir, QDirIterator::Subdirectories);
    while(iter.hasNext()) {
        const QFile file(iter.next());
        const QString filename = file.fileName();
        ui->statusbar->showMessage(QDir::toNativeSeparators(filename));
        ui->statusbar->repaint();

        bool duplicate = false;                 //don't add same file many times
        for(const auto &alreadyAddedFile : images_found)
            if(filename.toLower() == alreadyAddedFile.toLower()) {
                duplicate = true;
                break;
            }
        if(!duplicate) {
            Pic *picture = new Pic(this, filename);
            picture->setAutoDelete(false);
            thread_pool.start(picture);
            while(thread_pool.activeThreadCount() == thread_pool.maxThreadCount())
                QApplication::processEvents();          //avoid blocking signals in event loop
            images_found << filename;
        }
    }
}

void MainWindow::add_image_with_borders(Pic *add_me) {
    images_with_borders << add_me;

    QTableWidget *table = ui->images_table;
    table->insertRow ( table->rowCount() );
    table->setItem ( table->rowCount()-1, 0, new QTableWidgetItem( QDir::toNativeSeparators(add_me->filename) ));
}

void MainWindow::on_images_table_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous)
{
    Q_UNUSED(previous);
    if(images_with_borders.size() <= current->row())
        return;

    QLabel *label = ui->img_label;                                      //color separator won't show on 8-bit images
    QImage image = QImage(images_with_borders.at(current->row())->filename).convertToFormat(QImage::Format_RGB888);

    /* if a 1px separator is simply drawn on the image and image is then resized to fit the label,
     * the top or bottom separator lines can disappear because those exact rows can be lost during resizing.
     * therefore calculate how big image will be on screen and resize it before separator is drawn on it */
    const double image_resize_factor = std::min( double(label->height()) / image.height(),
                                                 double(label->width()) / image.width() );
    if(label->height() <= label->width())
        image = image.scaledToHeight(image.height() * image_resize_factor);
    else
        image = image.scaledToWidth(image.width() * image_resize_factor);

    const QRect rec(images_with_borders.at(current->row())->origin * image_resize_factor,
                    images_with_borders.at(current->row())->size * image_resize_factor);

    QPainter painter(&image);
    painter.setPen(QPen(Qt::green, 2, Qt::DashDotLine));
    painter.drawRect(rec);
    label->setPixmap(QPixmap::fromImage(image));
}
