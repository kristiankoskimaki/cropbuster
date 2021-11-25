#include "action.cpp"

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
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::add_rows);
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
    timer->stop(); delete timer; add_rows();    //ensure that remaining images are added when function ends
    ui->statusbar->showMessage(not_found);      //shows not found message or clears statusbar if no errors
}

void MainWindow::on_images_table_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous)
{
    Q_UNUSED(previous);
    if(images_with_borders.size() <= current->row())
        return;

    Pic *pic = images_with_borders.at(current->row());
    image = QImage(pic->filename).convertToFormat(QImage::Format_RGB888);   //color separator won't show on 8-bit images
    if (image.isNull()) {
        select_next_row(current->row());
        return;
    }
    image_height = image.height(); image_width = image.width();
    ui->open_in_explorer->setText(QFileInfo(pic->filename).fileName());
    draw_border_rectangle();
}

void MainWindow::draw_border_rectangle() {
    QLabel *label = ui->img_label;
                                        //label expands if painted with matching width/height pixmap
    QImage scaled_image = image.scaled( label->size() - QSize(2,2), Qt::KeepAspectRatio);
    QPainter painter(&scaled_image);
    painter.setPen(QPen(Qt::green, 1, Qt::DashDotLine));

    /* if a 1px separator is simply drawn on the image and image is then resized to fit the label,
     * the top or bottom separator lines can disappear because those exact rows can be lost during resizing.
     * therefore calculate how big image will be on screen and resize it before separator is drawn on it */
    const double image_resize_factor = std::min( double(label->height()) / image_height,
                                                 double(label->width()) / image_width );

    Pic *pic = images_with_borders.at(ui->images_table->currentRow());
    painter.drawRect(QRect(pic->origin * image_resize_factor, pic->size * image_resize_factor));
    label->setPixmap(QPixmap::fromImage(scaled_image));

    ui->about_image->setText(QStringLiteral("Image: %1 x %2\nSelection: %3 x %4 at (%5, %6)").
                             arg(image_width).arg(image_height).
                             arg(pic->size.width()).arg(pic->size.height()).arg(pic->origin.x()).arg(pic->origin.y()));
}

//using QTimer() to call add_rows() at regular intervals to populate filename table. event loop
//would otherwise fill up (=program frozen) if a new row was added every time an image with borders is found
void MainWindow::add_rows() {
    const int existing_rows = ui->images_table->rowCount();     //add new image filenames to table
    const int filenames_length = images_with_borders.size();    //since last time function was called

    for (int row=existing_rows; row<filenames_length; row++) {
        ui->images_table->insertRow(row);
        ui->images_table->setItem(row, 0, new QTableWidgetItem(QDir::toNativeSeparators(
                                              images_with_borders.at(row)->filename) ));
    }

    if (!existing_rows && filenames_length > 0) {               //focus on first filename added to list
        ui->images_table->selectRow(0);
        ui->images_table->setFocus();
    }
}
