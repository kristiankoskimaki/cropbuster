#include "mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::on_border_color_pref_clicked() {
    if (ui->border_color_pref->checkState() == Qt::Checked)
        border_preference = ONLY_BLACK_BORDER;
    else if (ui->border_color_pref->checkState() == Qt::Unchecked)
        border_preference = ANY_BORDER;
}

void MainWindow::on_open_in_explorer_clicked() {
    if (images_with_borders.empty())
        return;
    const QString filename = images_with_borders.at(ui->images_table->currentRow())->filename;

    #if defined(Q_OS_WIN)
        QProcess::startDetached(QStringLiteral("explorer"), QStringList {"/select,", QDir::toNativeSeparators(filename)});
    #endif
    #if defined(Q_OS_MACX)
        QProcess::startDetached(QStringLiteral("open"), QStringList {"-R", filename});
    #endif
    #if defined(Q_OS_X11)
        QProcess::startDetached(QStringLiteral("xdg-open"), QStringList {filename.left(filename.lastIndexOf("/"))});
    #endif
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
    }
    select_next_row(current_row);
}

void MainWindow::select_next_row(const int &current_row) {
    if (images_with_borders.count() > 1) {
        if (current_row == ui->images_table->rowCount())    //last row is currently selected, select previous row
            ui->images_table->selectRow(current_row - 1);
        else
            ui->images_table->selectRow(current_row + 1);   //there are rows below this one, select next row

    }
    else
        set_gui_state(DEACTIVATE_WIDGETS);                  //deactivate buttons since there are no images left in list

    images_with_borders.removeAt(current_row);
    ui->images_table->removeRow(current_row);
}

void MainWindow::on_grow_top_clicked() {
    if (!ui->images_table->rowCount())
        return;
    Pic* pic = images_with_borders.at(ui->images_table->currentRow());
    const int current_y = pic->origin.y();
    if (current_y == 0)
        return;

    pic->origin.setY(current_y - 1);
    pic->size.setHeight(pic->size.height() + 1);
    draw_border_rectangle();
}

void MainWindow::on_grow_bottom_clicked() {
    if (!ui->images_table->rowCount())
        return;
    Pic* pic = images_with_borders.at(ui->images_table->currentRow());
    if (image_height - pic->size.height() <= pic->origin.y())
        return;

    pic->size.setHeight(pic->size.height() + 1);
    draw_border_rectangle();
}

void MainWindow::on_grow_left_clicked() {
    if (!ui->images_table->rowCount())
        return;
    Pic* pic = images_with_borders.at(ui->images_table->currentRow());
    const int current_x = pic->origin.x();
    if (current_x == 0)
        return;

    pic->origin.setX(current_x - 1);
    pic->size.setWidth(pic->size.width() + 1);
    draw_border_rectangle();
}

void MainWindow::on_grow_right_clicked() {
    if (!ui->images_table->rowCount())
        return;
    Pic* pic = images_with_borders.at(ui->images_table->currentRow());
    if (image_width - pic->size.width() <= pic->origin.x())
        return;

    pic->size.setWidth(pic->size.width() + 1);
    draw_border_rectangle();
}

void MainWindow::on_shrink_top_clicked() {
    if (!ui->images_table->rowCount())
        return;
    Pic* pic = images_with_borders.at(ui->images_table->currentRow());
    const int height = pic->size.height();
    if (height <= 1)
        return;

    pic->origin.setY(pic->origin.y() + 1);
    pic->size.setHeight(height -1);
    draw_border_rectangle();
}

void MainWindow::on_shrink_bottom_clicked() {
    if (!ui->images_table->rowCount())
        return;
    Pic* pic = images_with_borders.at(ui->images_table->currentRow());
    const int height = pic->size.height();
    if (height <= 1)
        return;

    pic->size.setHeight(height -1);
    draw_border_rectangle();
}

void MainWindow::on_shrink_left_clicked() {
    if (!ui->images_table->rowCount())
        return;
    Pic* pic = images_with_borders.at(ui->images_table->currentRow());
    const int width = pic->size.width();
    if (width <= 1)
        return;

    pic->origin.setX(pic->origin.x() + 1);
    pic->size.setWidth(width - 1);
    draw_border_rectangle();
}

void MainWindow::on_shrink_right_clicked() {
    if (!ui->images_table->rowCount())
        return;
    Pic* pic = images_with_borders.at(ui->images_table->currentRow());
    const int width = pic->size.width();
    if (width <= 1)
        return;

    pic->size.setWidth(width - 1);
    draw_border_rectangle();
}
