#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    ui->folders_box->setFocus();
}

void MainWindow::on_scan_folders_clicked() {
    QStringList fixed_folders;
    QString not_found, typed_folders = ui->folders_box->text();
    const QRegularExpression re("\"|^ *| *$|\\*$|\\*\\.jpe?g$");        //quotes, leading/trailing space, *.jpeg

    for (const auto &typed : typed_folders.split(QStringLiteral(";"))) {
        QString folder = typed;
        folder.remove(re).replace(QRegularExpression(":+"), ":");       //remove double colons as well

        if(folder.isEmpty())
            continue;
        QString path = QFileInfo(folder).absoluteFilePath();
        if (!QFileInfo::exists(path) || path == QDir::currentPath())    //test if not found b/c no trailing slash
            path = QFileInfo(folder + QStringLiteral("/")).absoluteFilePath();
        if (!QFileInfo::exists(path) || path == QStringLiteral(":/")) {
            not_found = not_found + typed + QStringLiteral("  ");
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
    image = QImage(images_with_borders.at(current->row())->filename).convertToFormat(QImage::Format_RGB888);
    image_height = image.height(); image_width = image.width();

    /* if a 1px separator is simply drawn on the image and image is then resized to fit the label,
     * the top or bottom separator lines can disappear because those exact rows can be lost during resizing.
     * therefore calculate how big image will be on screen and resize it before separator is drawn on it */
    const double image_resize_factor = std::min( double(label->height()) / image_height,
                                                 double(label->width()) / image_width );

    image = image.scaled(label->size(), Qt::KeepAspectRatio);
    const QRect rec(images_with_borders.at(current->row())->origin * image_resize_factor,
                    images_with_borders.at(current->row())->size * image_resize_factor);

    QPainter painter(&image);
    painter.setPen(QPen(Qt::green, 2, Qt::DashDotLine));
    painter.drawRect(rec);
    label->setPixmap(QPixmap::fromImage(image).scaled( QSize( label->width()-2, label->height()-2 ),
                     Qt::KeepAspectRatio));     //label expands if painted with matching width/height pixmap
    ui->statusbar->clearMessage();
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
    if (!existing_rows && table->rowCount() > 0) {
        table->selectRow(0);
        table->setFocus();
    }
}

void MainWindow::on_save_as_clicked() {
    if (!ui->images_table->rowCount())
        return;

    const int current_row = ui->images_table->currentRow();
    const Pic* picture = images_with_borders.at(current_row);   //crop the image from inside the border
    const QImage image = QImage(picture->filename).copy(picture->origin.x(), picture->origin.y(),
                                                        picture->size.width(), picture->size.height());
    //save as dialog with current filename already selected
    const QString save_filename = QFileDialog::getSaveFileName(this,
                                  QStringLiteral("Save as"), picture->filename, QStringLiteral("JPEG files(*.jpg)"));
    if (save_filename.isEmpty())
        return;
    image.save(save_filename, "JPG", ui->jpg_quality->value());
    if (!QFileInfo::exists(save_filename)) {
        ui->statusbar->showMessage(QStringLiteral("Error saving ") + QDir::toNativeSeparators(save_filename));
        return;
    }

    select_next_row(current_row);
}

void MainWindow::select_next_row(const int &current_row) {
    ui->images_table->setRowHidden(current_row, true);

    int select_next_row = -1;                       //find next visible row
    for (int i=current_row; i<ui->images_table->rowCount(); i++) {
        if (!ui->images_table->isRowHidden(i)) {
            select_next_row = i;
            break;
        }
    }                                               //none found, find previous visible row
    if (select_next_row == -1) {
        for (int i=current_row; i>=0; i--) {
            if (!ui->images_table->isRowHidden(i)) {
                select_next_row = i;
                break;
            }
        }
    }

    if (select_next_row != -1)
        ui->images_table->selectRow(select_next_row);
    else {
        ui->img_label->setPixmap(QPixmap());        //there were no other visible rows, clear image
        ui->save_as->setDisabled(true);
    }
}
