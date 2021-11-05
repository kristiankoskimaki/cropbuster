#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    ui->folders_box->setFocus();
}

void MainWindow::on_scan_folders_clicked() {
    QStringList fixed_folders;
    QString not_found, typed_folders = ui->folders_box->text().replace(";:+;", ";");
    typed_folders.remove(QRegularExpression("\"|^ *| +$|\\*\\.?j?p?e?g?$"));    //remove colon, quotes, extra space, .jpg

    for (auto &folder : typed_folders.split(QStringLiteral(";"))) {
        if (folder == QStringLiteral("c:") || folder == QStringLiteral("C:"))
            folder = QStringLiteral("c:\\");                //bug? "c:" -> Qt debug folder, but "d:" -> "d:\"
        const QString path = QFileInfo(folder).absoluteFilePath();
        if (!QFileInfo::exists(path)) {
            not_found = not_found + folder + QStringLiteral("  ");
            continue;
        }
        bool add_this = true;                                //eliminate duplicate folders in list
        for (int i=fixed_folders.size()-1; i>=0; i--) {
            if (path.indexOf(fixed_folders.at(i)) == 0) {    //duplicate OR parent already added
                add_this = false;
                continue;
            }
            if (fixed_folders.at(i).indexOf(path) == 0)      //subfolder found in list, delete it
                fixed_folders.remove(i);
        }
        if (add_this)
            fixed_folders << path;
    }

    if(not_found != QStringLiteral(""))
        not_found = QStringLiteral("Can't find ") + not_found;
    search_for_images(fixed_folders, not_found);
}

void MainWindow::search_for_images(const QStringList &folders, const QString &not_found) {
    ImageTable image_table(ui->images_table, &images_with_borders);
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, &image_table, &ImageTable::add_rows);
    timer->start(1000);         //program is responsive if filenames are added to table at intervals

    ui->statusbar->showMessage(not_found);
    QThreadPool pool;           //for multithreading, create threadpool for all Pic() objects

    for (auto &folder : folders) {
        QDirIterator iter(QDir(folder, QStringLiteral("*.jp*g"), QDir::NoSort), QDirIterator::Subdirectories);
        while (iter.hasNext()) {
            while(pool.activeThreadCount() == pool.maxThreadCount())    //1. don't flood event loop with instances
                QApplication::processEvents();                          //2. avoid blocking signals in event loop
            if (stop_scanning) {
                pool.clear(); return;               //stop creating threads when force quit program
            }
            const QString filename = QFile(iter.next()).fileName();
            Pic *picture = new Pic(this, filename); //important! many instances of same class in threadpool crashes
            picture->setAutoDelete(false);          //(because some objects get deleted) without this (race condition)
            pool.start(picture);                    //every instances Pic::run() is executed when free thread available
            ui->statusbar->showMessage(not_found + QDir::toNativeSeparators(filename));
        }
    }

    pool.waitForDone();
    QApplication::processEvents();              //process signals from last threads
    image_table.add_rows();                     //ensure that remaining images are added when function ends
    ui->statusbar->showMessage(not_found);      //shows not found message or clears statusbar if no errors
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

    image = image.scaled(label->size(), Qt::KeepAspectRatio);
    const QRect rec(images_with_borders.at(current->row())->origin * image_resize_factor,
                    images_with_borders.at(current->row())->size * image_resize_factor);

    QPainter painter(&image);
    painter.setPen(QPen(Qt::green, 2, Qt::DashDotLine));
    painter.drawRect(rec);
    label->setPixmap(QPixmap::fromImage(image).scaled( QSize( label->width()-2, label->height()-2 ),
                     Qt::KeepAspectRatio));     //label expands if painted with matching width/height pixmap
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event)
    if(images_with_borders.empty())
        return;

    QTableWidgetItem *selected_row = ui->images_table->selectedItems()[0];
    on_images_table_currentItemChanged(selected_row, selected_row);
}


void ImageTable::add_rows() {
    const int existing_rows = table->rowCount();        //add new image filenames to table
    const int filenames_length = filenames->size();     //since last time function was last called

    for (int i=existing_rows; i<filenames_length; i++) {
        table->insertRow ( table->rowCount() );
        table->setItem ( table->rowCount()-1, 0, new QTableWidgetItem(
                         QDir::toNativeSeparators(filenames->at(i)->filename) ));
    }
    if (filenames->size() == 1)     //show first image found on screen
        table->selectRow(0);
}
