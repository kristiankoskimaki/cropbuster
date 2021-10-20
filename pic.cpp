#include "pic.h"

Pic::Pic(QObject *_mainwPtr, const QString &filenameParam) : filename(filenameParam)
{
    QObject::connect(this, SIGNAL(add_this_image(Pic*)), _mainwPtr, SLOT(add_image_with_borders(Pic*)));
}

void Pic::run()
{
    if(QFileInfo::exists(filename) && findFrame())
        emit add_this_image(this);
    else
        this->setAutoDelete(true);
}

bool Pic::findFrame() {
    using namespace cv;
    Mat gray_image = imread(filename.toLocal8Bit().toStdString(), IMREAD_GRAYSCALE);
    if (!gray_image.dims) return false;         //could not open image, eg. special character in filename

    threshold(gray_image, gray_image, 230, 0, THRESH_TOZERO_INV);   //make dark/bright pixels (border?) solid black
    threshold(gray_image, gray_image, 25, 0, THRESH_TOZERO);        //this makes finding presumptive border easier

    int largest_area = 0;
    std::vector<Point> largest_contour;
    std::vector<std::vector<Point>> contours;
    findContours(gray_image, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);     //find contours (shapes in image)

    for (const auto& contour : contours) {
        std::vector<Point> vertices;
        approxPolyDP(contour, vertices, arcLength(contour, true) * 0.01, true);
        if (vertices.size() != 4) continue;         //not a rectangle, ignore this contour

        const double& area = contourArea(contour);  //looking for the largest rectangle in image
        if (area > largest_area) {
            largest_area = area;
            largest_contour = contour;
        }
    }

    const double& image_to_border_ratio = double(largest_area) / gray_image.total();
    if (image_to_border_ratio < 0.10 || image_to_border_ratio > 0.90)
        return false;   //too small: only minor detail detected, too large: there is no border

    const Rect& bounding_rect = boundingRect(largest_contour);
    origin.setX(bounding_rect.x); size.setHeight(bounding_rect.height - 1);
    origin.setY(bounding_rect.y); size.setWidth(bounding_rect.width - 1);

    return true;
}
