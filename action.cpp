#include "mainwindow.h"
#include "ui_mainwindow.h"

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
