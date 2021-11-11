#include "pic.h"

Pic::Pic(QObject *_mainwPtr, const QString &filenameParam) : filename(filenameParam)
{
    QObject::connect(this, SIGNAL(add_this_image(Pic*)), _mainwPtr, SLOT(add_image_with_borders(Pic*)));
}

void Pic::run()
{
    using namespace cv;
    Mat gray_image = imread(filename.toLocal8Bit().toStdString(), IMREAD_GRAYSCALE);
    if (!gray_image.dims) {                     //could not open image, eg. special character in filename
        this->setAutoDelete(true);
        return;
    }

    int largest_area = 0;
    Rect largest_rect;
    std::vector<std::vector<Point>> contours;
    threshold(gray_image, gray_image, 0, 255, THRESH_BINARY);       //2 colors only, all details removed
    findContours(gray_image, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);     //find shapes in image

    for (const auto& contour : contours) {
        const Rect bounding_rect = boundingRect(contour);
        const int area = bounding_rect.width * bounding_rect.height;
        if (area > largest_area) {                                              //find largest rectangle
            largest_area = area;
            largest_rect = bounding_rect;
        }
    }

    if (double(largest_area) / gray_image.total() > 0.95) {                     //no border in image
        this->setAutoDelete(true);
        return;
    }
    origin.setX(largest_rect.x); size.setHeight(largest_rect.height);
    origin.setY(largest_rect.y); size.setWidth(largest_rect.width);
    emit add_this_image(this);
}
