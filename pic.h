#ifndef PIC_H
#define PIC_H

#include <opencv2/opencv.hpp>
#include <QObject>
#include <QRunnable>
#include <QSize>
#include <QPoint>
#include "prefs.h"

class Pic: public QObject, public QRunnable
{
    Q_OBJECT

public:
    Pic(const QString &filenameParam, const Prefs &prefsParam);
    void run();
    bool find_border_color(cv::Mat &image, uchar &border_color);
    int most_frequent_array_color(QVector<uchar> &pixels, uchar &border_color);
    void find_exact_edges(cv::Mat &image, cv::Rect &rectangle, uchar &border_color);

    QString filename;
    QSize size;
    QPoint origin = {0, 0};

signals:
    void add_this_image(Pic*add_me);

private:
    Prefs prefs;
    //true (default) will find most images where there is an actual black border around a photo image.
    //false finds finds more images, both those with another color border, but also images that do not
    //really have borders (single color background, broken jpgs...)
    static constexpr bool ONLY_BLACK_BORDER = true;   //true: black border only. false: border color = outer edge pixels
};

#endif // PIC_H
