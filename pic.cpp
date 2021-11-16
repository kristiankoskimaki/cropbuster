#include "pic.h"
using namespace cv;

Pic::Pic(QObject *_mainwPtr, const QString &filenameParam) : filename(filenameParam)
{
    QObject::connect(this, SIGNAL(add_this_image(Pic*)), _mainwPtr, SLOT(add_image_with_borders(Pic*)));
}

void Pic::run()
{
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

    find_exact_edges(largest_rect);
    origin.setX(largest_rect.x); size.setHeight(largest_rect.height);
    origin.setY(largest_rect.y); size.setWidth(largest_rect.width);
    emit add_this_image(this);
}

//thresholding image makes finding edges easier, but also distorts it: rectangle is a few pixels off
void Pic::find_exact_edges(cv::Rect &rect) {
    Mat image = imread(filename.toLocal8Bit().toStdString(), IMREAD_GRAYSCALE);

    uchar border_color;                         //border color selected 1px outside center image...
    if (rect.y > 0)
        border_color = *image.ptr<uchar>(rect.y-1, rect.x);                 //...from border on top
    else if (rect.x > 0)
        border_color = *image.ptr<uchar>(rect.y, rect.x-1);                 //...from border on left
    else if (image.rows - rect.height > 0)
        border_color = *image.ptr<uchar>(rect.y + rect.height, rect.x);     //...from border on bottom
    else
        border_color = *image.ptr<uchar>(rect.y, rect.x + rect.width);      //...from border on right

    const int border_threshold = 35;                            //border/image color difference
    int max_deviation = min(rect.width/2, rect.height/2);
    max_deviation = min(max_deviation, 30);                     //don't seek more than 30px for edge

    for (int row=0; row<max_deviation; row++) {             //top edge
        uchar* pixel = image.ptr<uchar>(rect.y+row, rect.x);
        uchar* end = pixel + rect.width;
        do {                                    //compare every pixel in row/column to background color
            if ( abs( *pixel - border_color) > border_threshold) {
                rect.y += row;                  //if pixel differs enough, we found edge: adjust rect
                rect.height -= row;
                row = max_deviation; break;
            }
        } while (pixel++ < end);
    }

    for (int row=0; row<max_deviation; row++) {             //bottom edge
        uchar* pixel = image.ptr<uchar>( rect.y + rect.height-1 - row, rect.x);
        uchar* end = pixel + rect.width;
        do {
            if ( abs( *pixel - border_color) > border_threshold) {
                rect.height -= row;
                row = max_deviation; break;
            }
        } while (pixel++ < end);
    }

    for (int col=0; col<max_deviation; col++) {             //left edge
        uchar* pixel = image.ptr<uchar>(rect.y, rect.x + col);
        uchar* end = image.ptr<uchar>( rect.y + rect.height-1, rect.x + col);
        do {
            if ( abs( *pixel - border_color) > border_threshold) {
                rect.x += col;
                rect.width -= col;
                col = max_deviation; break;
            }
            pixel += image.cols;
        } while (pixel < end);
    }

    for (int col=0; col<max_deviation; col++) {             //right edge
        uchar* pixel = image.ptr<uchar>( rect.y, rect.x + rect.width-1 - col);
        uchar* end = image.ptr<uchar>( rect.y + rect.height-1, rect.x + rect.width-1 - col);
        do {
            if ( abs( *pixel - border_color) > border_threshold) {
                rect.width -= col;
                col = max_deviation; break;
            }
            pixel += image.cols;
        } while (pixel < end);
    }
}
