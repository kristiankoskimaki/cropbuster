#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    ui->folders_box->setFocus();
    QTableWidget *table = ui->images_table;
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void MainWindow::on_scan_folders_clicked() {
    QString not_found, typed_folders = ui->folders_box->text().remove(QRegularExpression("\"|^ *| +$|\\*\\.?j?p?e?g?$"));
    QStringList fixed_folders;                              //remove quotes, leading and trailing space, .jpg extension

    for (auto &folder : typed_folders.split(QStringLiteral(";"))) {
        QFileInfo file = QFileInfo(folder);
        const QString filename = file.fileName();
        const QString path = file.absoluteFilePath();

        if (filename == QStringLiteral("") ||                               //"c:" is read as "c:/[a root folder]"
            filename.at(filename.size()-1) != '/' )                         //"d:" is ok, as is "c:/" (bug?)
            file = QFileInfo(folder + QStringLiteral("/"));
        if (folder == QStringLiteral("") || folder == QStringLiteral(":"))
            continue;
        if (file.absolutePath() == QDir::currentPath() || !QFileInfo::exists(path)) {
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
    QThreadPool pool;

    for(auto &folder : folders) {
        QDirIterator iter(QDir(folder, QStringLiteral("*.jp*g"), QDir::NoSort), QDirIterator::Subdirectories);
        while(iter.hasNext()) {
            if(stop_scanning)
                return;
            const QString filename = QFile(iter.next()).fileName();
            ui->statusbar->showMessage(not_found + QDir::toNativeSeparators(filename));
            Pic *picture = new Pic(this, filename);
            picture->setAutoDelete(false);
            pool.start(picture);
            QApplication::processEvents();          //avoid blocking signals in event loop
        }
        pool.waitForDone();
        QApplication::processEvents();              //process signals from last threads
        ui->statusbar->clearMessage();
    }
}

void MainWindow::add_image_with_borders(Pic *add_me) {
    images_with_borders << add_me;
    QTableWidget *table = ui->images_table;
    table->insertRow ( table->rowCount() );
    table->setItem ( table->rowCount()-1, 0, new QTableWidgetItem( QDir::toNativeSeparators(add_me->filename) ));

    if(images_with_borders.size() == 1)     //first image found, show it on screen
        table->selectRow(0);
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
