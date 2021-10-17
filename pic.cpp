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

    Mat clear_image, sans_frame, gray_image = imread(filename.toLocal8Bit().toStdString(), IMREAD_GRAYSCALE);
    if(!gray_image.dims)
        return false;

    threshold(gray_image, sans_frame, 249, 0, THRESH_TOZERO_INV);   //remove near white pixels
    threshold(sans_frame, sans_frame, 5, 0, THRESH_TOZERO);         //near black pixels as well
    if( countNonZero(sans_frame) / double(sans_frame.rows * sans_frame.cols) > 0.9)
        return false;           //image has no border since there are too few black/white pixels

    GaussianBlur(gray_image, gray_image, Size(5, 5), 0, 0);         //fewer complex contours after blurring a bit
    threshold(gray_image, clear_image, 150, 255, THRESH_BINARY);    //remove dark pixels to find contours better

    std::vector<std::vector<Point>> contours;       //find biggest contour in image (=picture inside frame)
    findContours(clear_image, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    int largest_area = 0, largest_contour = 0;

    for( uint i = 0; i < contours.size(); i++ ) {   //iterate through each contour
        double area = contourArea( contours[i] );   //find the area of contour
        if( area > largest_area ) {
            largest_area = area;
            largest_contour = i;                    //store the index of largest contour
        }
    }

    if(contours.empty())
        return false;
    Rect bounding_rect = boundingRect(contours[largest_contour]);
    origin.setX(bounding_rect.x); size.setHeight(bounding_rect.height);
    origin.setY(bounding_rect.y); size.setWidth(bounding_rect.width);
    return true;
}
