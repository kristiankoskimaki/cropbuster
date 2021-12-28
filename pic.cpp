#include "pic.h"
using namespace cv;

Pic::Pic(QObject *_mainwPtr, const QString &filenameParam, const bool &border_pref) :
                             filename(filenameParam), border_preference(border_pref) {
    QObject::connect(this, SIGNAL(add_this_image(Pic*)), _mainwPtr, SLOT(add_image_with_borders(Pic*)));
}

void Pic::run()
{
    Mat gray_image_original, gray_image = imread(filename.toLocal8Bit().toStdString(), IMREAD_GRAYSCALE);
    if (gray_image.total() == 0 || gray_image.total() > 33177600) {
        this->setAutoDelete(true);                      //special character in filename or >8K (too big for QImage)
        return;
    }
    gray_image.copyTo(gray_image_original);             //thresholding modifies image, make copy for reading exact pixels later

    uchar border_color = 0;
    if (border_preference != ONLY_BLACK_BORDER) {
        if (!find_border_color(gray_image, border_color)) {                     //disregard images without a border
            this->setAutoDelete(true);
            return;
        }                                               //image must be simplified to pure black/white in order for opencv
        uchar *pixel = gray_image.data;                 //to easily find correct border and its position. two methods are used:
        while (pixel++ < gray_image.dataend - 1) {      //arbitrary border color: turn all pixels with the border color to black,
            if (*pixel == border_color)                 //and all other pixels to white
                *pixel = 0;
            else
                *pixel = 255;
        }
    }
    else                                                                //for black border detection only: use algorithm to turn
        threshold(gray_image, gray_image, 0, 255, THRESH_BINARY);       //dark pixels pure black and bright pixels pure white

    int largest_area = 0;
    Rect largest_rect;
    std::vector<std::vector<Point>> contours;
    findContours(gray_image, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);     //find contours (shapes) in image

    for (const auto& contour : contours) {
        const Rect bounding_rect = boundingRect(contour);                       //examine rectangles surrounding contour edge points
        const int area = bounding_rect.width * bounding_rect.height;
        if (area > largest_area) {                                              //find largest rectangle (should be the border)
            largest_area = area;
            largest_rect = bounding_rect;
        }
    }

    if (double(largest_area) / gray_image.total() > MAX_BORDER_PERCENT) {       //if there is no border in image, then the largest
        this->setAutoDelete(true);                                              //detected shape will be the image itself. disregard
        return;
    }

    find_exact_edges(gray_image_original, largest_rect, border_color);
    origin.setX(largest_rect.x); size.setHeight(largest_rect.height);
    origin.setY(largest_rect.y); size.setWidth(largest_rect.width);
    emit add_this_image(this);
}

//find most common color (likely the border color) from outside edge pixels
//return true if image has border (enough pixels of the same color found), otherwise return false
bool Pic::find_border_color(Mat &image, uchar &border_color) {
    const uchar *topright = image.data + image.cols;
    const uchar *bottomleft = image.ptr(image.rows - 1);
    const uchar *toprow  = image.data; const uchar *bottomrow = bottomleft;
    const uchar *leftcol = image.data; const uchar *rightcol = topright;

    const int skip_pixels = SKIP_EDGE_PIXELS;
    QVector<uchar> pixels(0);
    while (toprow < topright) {                 //copy pixels from top and bottom row
        pixels << *toprow;
        pixels << *bottomrow;
        toprow += skip_pixels;
        bottomrow += skip_pixels;
    }
    while (leftcol < bottomleft) {              //copy pixels from first and last column
        pixels << *leftcol;
        pixels << *rightcol;
        leftcol += skip_pixels * image.cols;
        rightcol += skip_pixels * image.cols;
    }

    int max_count = most_frequent_array_color(pixels, border_color);
    if (max_count / double(pixels.size()) < EDGE_BORDER_RATIO)
        return false;
    return true;
}

int Pic::most_frequent_array_color(QVector<uchar> &pixels, uchar &border_color) {
    std::sort(pixels.begin(), pixels.end());
    int max_count = 1,  curr_count = 1;
    border_color = pixels[0];

    for (int i=1; i<pixels.size(); i++) {       //find most common color (border?)
        if (pixels[i] == pixels[i - 1])
            curr_count++;
        else {
            if (curr_count > max_count) {
                max_count = curr_count;
                border_color = pixels[i - 1];
            }
            curr_count = 1;
        }
    }
    if (curr_count > max_count) {               //check for last element in array, not handled inside for() loop
        max_count = curr_count;
        border_color = pixels[pixels.size() - 1];
    }

    return max_count;
}

//thresholding image makes finding edges easier, but also distorts it: rectangle is a few pixels off
void Pic::find_exact_edges(Mat &image, Rect &rect, uchar &border_color) {
    const int max_deviation = min( min(rect.width/2, rect.height/2), DEFAULT_DEVIATION);

    if (border_preference == ONLY_BLACK_BORDER) {           //border color selected 1px outside center image...
        if (rect.y > 0)
            border_color = *image.ptr<uchar>(rect.y-1, rect.x);                 //...from border on top
        else if (rect.x > 0)
            border_color = *image.ptr<uchar>(rect.y, rect.x-1);                 //...from border on left
        else if (image.rows - rect.height > 0)
            border_color = *image.ptr<uchar>(rect.y + rect.height, rect.x);     //...from border on bottom
        else
            border_color = *image.ptr<uchar>(rect.y, rect.x + rect.width);      //...from border on right
    }

    for (int row=0; row<max_deviation; row++) {             //top edge
        uchar* pixel = image.ptr<uchar>(rect.y+row, rect.x);
        uchar* end = pixel + rect.width;
        int non_border_pixels = 0;
        do                                  //compare every pixel in row/column to background color
            if ( abs( *pixel - border_color) > BORDER_THRESHOLD)
                non_border_pixels++;
        while (pixel++ < end);

        if (non_border_pixels > NOT_A_BORDER) {
            rect.y += row;                  //if pixels differ enough, we found edge: adjust rect
            rect.height -= row;
            break;
        }
    }

    for (int row=0; row<max_deviation; row++) {             //bottom edge
        uchar* pixel = image.ptr<uchar>( rect.y + rect.height-1 - row, rect.x);
        uchar* end = pixel + rect.width;
        int non_border_pixels = 0;
        do
            if ( abs( *pixel - border_color) > BORDER_THRESHOLD)
                non_border_pixels++;
        while (pixel++ < end);

        if (non_border_pixels > NOT_A_BORDER) {
            rect.height -= row;
            break;
        }
    }

    for (int col=0; col<max_deviation; col++) {             //left edge
        uchar* pixel = image.ptr<uchar>(rect.y, rect.x + col);
        uchar* end = image.ptr<uchar>( rect.y + rect.height-1, rect.x + col);
        int non_border_pixels = 0;
        do {
            if ( abs( *pixel - border_color) > BORDER_THRESHOLD)
                non_border_pixels++;
            pixel += image.cols;
        } while (pixel < end);

        if (non_border_pixels > NOT_A_BORDER) {
            rect.x += col;
            rect.width -= col;
            break;
        }
    }

    for (int col=0; col<max_deviation; col++) {             //right edge
        uchar* pixel = image.ptr<uchar>( rect.y, rect.x + rect.width-1 - col);
        uchar* end = image.ptr<uchar>( rect.y + rect.height-1, rect.x + rect.width-1 - col);
        int non_border_pixels = 0;
        do {
            if ( abs( *pixel - border_color) > BORDER_THRESHOLD)
                non_border_pixels++;
            pixel += image.cols;
        } while (pixel < end);

        if (non_border_pixels > NOT_A_BORDER) {
            rect.width -= col;
            break;
        }
    }
}
