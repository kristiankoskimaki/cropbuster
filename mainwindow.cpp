#include "action.cpp"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    ui->folders_box->setFocus();
    ui->browse_folders->setIcon(ui->browse_folders->style()->standardIcon(QStyle::SP_DirOpenIcon));
    ui->thread_limiter->setMaximum(QThread::idealThreadCount());
    ui->thread_limiter->setValue(QThread::idealThreadCount());

    //every time [vert/hor]ScrollBar is moved, copy the new position to mainWindow's [vert/hor]_scrollbar_pos member variable
    connect( ui->scrollArea->verticalScrollBar(), &QScrollBar::valueChanged, [this](int pos) {
             vertical_scrollbar_pos = double(pos) / ui->scrollArea->verticalScrollBar()->maximum(); } );
    connect( ui->scrollArea->horizontalScrollBar(), &QScrollBar::valueChanged, [this](int pos) {
             horizontal_scrollbar_pos = double(pos) / ui->scrollArea->horizontalScrollBar()->maximum(); } );

}

void MainWindow::dropEvent(QDropEvent *event) {
    const QString file_name = event->mimeData()->urls().first().toLocalFile();
    const QFileInfo file(file_name);
    if(file.isDir())
        ui->folders_box->insert(QStringLiteral(";%1").arg(QDir::toNativeSeparators(file_name)));
}

void MainWindow::set_gui_state(const int &state) {
    if (state == PAUSE_WIDGETS) {
        ui->border_color_pref->setDisabled(true);
        ui->thread_limiter->setDisabled(true);
        ui->progress_bar->setValue(0);
        ui->progress_bar->setMaximum(INT_MAX);
    }
    if (state == UNPAUSE_WIDGETS) {
        ui->border_color_pref->setDisabled(false);
        ui->thread_limiter->setDisabled(false);
        ui->zoom->setDisabled(false);
    }
    if (state == ACTIVATE_WIDGETS) {
        ui->open_in_explorer->setDisabled(false);
        ui->grow_top->setDisabled(false);    ui->shrink_top->setDisabled(false);
        ui->grow_bottom->setDisabled(false); ui->shrink_bottom->setDisabled(false);
        ui->grow_left->setDisabled(false);   ui->shrink_left->setDisabled(false);
        ui->grow_right->setDisabled(false);  ui->shrink_right->setDisabled(false);
        ui->save_as->setDisabled(false);
        ui->zoom->setDisabled(false);
        ui->scrollArea->setWidget(new QLabel);
        ui->images_table->setRowCount(0);
    }
    if (state == DEACTIVATE_WIDGETS) {
        ui->about_image->clear();
        ui->open_in_explorer->clear();
        ui->open_in_explorer->setDisabled(true);
        ui->grow_top->setDisabled(true);    ui->shrink_top->setDisabled(true);
        ui->grow_bottom->setDisabled(true); ui->shrink_bottom->setDisabled(true);
        ui->grow_left->setDisabled(true);   ui->shrink_left->setDisabled(true);
        ui->grow_right->setDisabled(true);  ui->shrink_right->setDisabled(true);
        ui->save_as->setDisabled(true);
        ui->zoom->setDisabled(true);
        ui->scrollArea->setWidget(new QLabel);
    }
}

void MainWindow::on_browse_folders_clicked() {
    const QString dir = QFileDialog::getExistingDirectory(nullptr, QByteArrayLiteral("Open folder"), QStringLiteral("/"),
                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(dir.isEmpty())
        return;
    ui->folders_box->insert(QStringLiteral(";%1").arg(QDir::toNativeSeparators(dir)));
    ui->folders_box->setFocus();
}

void MainWindow::on_scan_folders_clicked() {
    if(!images_with_borders.empty()) {
        if (QMessageBox::Yes == QMessageBox(QMessageBox::Information, "Rescan", "Discard results and search again?",
                                QMessageBox::Yes|QMessageBox::No).exec())
            images_with_borders.clear();
        else
            return;
    }
    set_gui_state(ACTIVATE_WIDGETS);

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
    ui->statusbar->showMessage(not_found);

    set_gui_state(PAUSE_WIDGETS);
    search_for_images(fixed_folders, not_found);
}

void MainWindow::search_for_images(const QStringList &folders, const QString &not_found) {
    //to set the progress bar max value, search through folders for all jpgs in background thread
    QFuture<void> future = QtConcurrent::run(&MainWindow::get_progressbar_max, this, folders);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::add_rows);
    timer->start(1000);         //program is responsive if filenames are added to table at intervals

    Prefs prefs(this);
    prefs.BORDER_SETTING = border_preference;

    QThreadPool pool;           //for multithreading, create threadpool for all Pic() objects
    pool.setMaxThreadCount(ui->thread_limiter->value());

    for (auto &folder : folders) {
        QDirIterator iter(QDir(folder, QStringLiteral("*.jp*g"), QDir::NoSort), QDirIterator::Subdirectories);
        while (iter.hasNext()) {
            while(pool.activeThreadCount() == pool.maxThreadCount())    //1. don't flood event loop with instances
                QApplication::processEvents();                          //2. avoid blocking signals in event loop
            if (stop_scanning) {
                pool.clear(); return;               //stop creating threads when force quit program
            }
            const QString filename = QFile(iter.next()).fileName();
            //Pic *picture = new Pic(this, filename, border_preference);
            Pic *picture = new Pic(filename, prefs);
            picture->setAutoDelete(false);          //important! many instances of same class in threadpool crashes
                                                    //(because some objects get deleted) without this (race condition)
            pool.start(picture);                    //every instances Pic::run() is executed when free thread available
            ui->progress_bar->setValue(ui->progress_bar->value() + 1);
            ui->statusbar->showMessage(not_found + QDir::toNativeSeparators(filename));
        }
    }

    pool.waitForDone();
    QApplication::processEvents();              //process signals from last threads
    timer->stop(); delete timer; add_rows();    //ensure that remaining images are added when function ends
    future.waitForFinished();                   //wait until finished (crash when going out of scope destroys instance)
    set_gui_state(UNPAUSE_WIDGETS);

    ui->statusbar->showMessage(not_found);      //shows not found message or clears statusbar if no errors
    if (not_found.isEmpty() && images_with_borders.isEmpty())
        ui->statusbar->showMessage(QStringLiteral("No image files found"));
}

void MainWindow::get_progressbar_max(const QStringList &folders) {
    int files_found = 0;
    for (auto &folder : folders) {
        QDirIterator iter(QDir(folder, QStringLiteral("*.jp*g"), QDir::NoSort), QDirIterator::Subdirectories);
        while (iter.hasNext()) {
            files_found++;
            iter.next();
        }
    }
    if (!files_found)
        return;

    //this function is running in another thread and should not modify gui widgets by itself
    //instead, emit signal to a function in the main gui thread to do that
    QObject::connect(this, SIGNAL(show_progressbar(int)), this, SLOT(set_progressbar_max(int)));
    emit show_progressbar(files_found);
}

void MainWindow::set_progressbar_max(const int &max) {
    ui->progress_bar->setMaximum(max);
}

void MainWindow::on_images_table_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous)
{
    Q_UNUSED(previous);
    if(!current || images_with_borders.size() <= current->row())
        return;

    Pic *pic = images_with_borders.at(current->row());
    image = QImage(pic->filename).convertToFormat(QImage::Format_RGB888);   //color separator won't show on 8-bit images
    if (image.isNull()) {
        select_next_row(current->row());
        return;
    }
    image_height = image.height(); image_width = image.width();
    vertical_scrollbar_pos = 0.0; horizontal_scrollbar_pos = 0.0;           //reset scrollbars when the image changes
    draw_border_rectangle();
}

void MainWindow::draw_border_rectangle() {
    if(images_with_borders.empty())
        return;

    const Pic *pic = images_with_borders.at(ui->images_table->currentRow());
    QImage scaled_image = image;
    const QSize size_on_screen = ui->scrollArea->maximumViewportSize();

    double resize_factor = 1.0;
    /* if a 1px separator is simply drawn on the image and image is then resized to fit the label,
     * the top or bottom separator lines can disappear because those exact rows can be lost during resizing.
     * therefore calculate how big image will be on screen and resize it before separator is drawn on it */
    if (show_scaled_image == true) {
        resize_factor = std::min( size_on_screen.height() / double(image_height),
                                  size_on_screen.width() / double(image_width));
        scaled_image = image.scaled(size_on_screen, Qt::KeepAspectRatio);
    }

    const int sel_x =          qFloor( resize_factor * pic->origin.x()    );
    const int sel_y =          qFloor( resize_factor * pic->origin.y()    );
    const int sel_w = std::min(qFloor( resize_factor * pic->size.width()  ),
                                       scaled_image.width() - sel_x - 1   );
    const int sel_h = std::min(qFloor( resize_factor * pic->size.height() ),
                                       scaled_image.height() - sel_y - 1  );

    QPainter painter(&scaled_image);
    painter.setPen(QPen(Qt::green, 1, Qt::DashDotLine));
    painter.drawRect(QRect( QPoint(sel_x, sel_y), QSize(sel_w, sel_h) ));
    QLabel *label = new QLabel;
    label->setPixmap(QPixmap::fromImage(scaled_image));

    //every time the selection rectangle is resized, the image is redrawn with the new rectangle on top of it.
    //when the image is redrawn though, the scrollbars reset to upper left corner, i.e. [0,0] position.
    //unless fixed, you would have to manually scroll back every time after you clicked a resizing button
    const double hscroll = horizontal_scrollbar_pos;   //must copy values now, since setWidget() will scroll back to
    const double vscroll = vertical_scrollbar_pos;     //upper left corner, changing xxx_scrollbar_pos to 0
    ui->scrollArea->setWidget(label);
    ui->scrollArea->verticalScrollBar()->setValue(vscroll * ui->scrollArea->verticalScrollBar()->maximum());
    ui->scrollArea->horizontalScrollBar()->setValue(hscroll * ui->scrollArea->horizontalScrollBar()->maximum());

    ui->open_in_explorer->setText(QFileInfo(pic->filename).fileName());
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
