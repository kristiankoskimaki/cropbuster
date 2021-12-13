#ifndef PIC_H
#define PIC_H

#include <opencv2/opencv.hpp>
#include <QObject>
#include <QRunnable>
#include <QSize>
#include <QPoint>

class Pic: public QObject, public QRunnable
{
    Q_OBJECT

public:
    Pic(QObject *_mainwPtr, const QString &filenameParam, const bool &border_pref);
    void run();
    bool find_border_color(cv::Mat &image, uchar &border_color);
    int most_frequent_array_color(QVector<uchar> &pixels, uchar &border_color);
    void find_exact_edges(cv::Mat &image, cv::Rect &rectangle, uchar &border_color);

    QString filename;
    QSize size;
    QPoint origin = {0, 0};
    bool border_preference;

signals:
    void add_this_image(Pic*add_me);

private:
    //true (default) will find most images where there is an actual black border around a photo image.
    //false finds finds more images, both those with another color border, but also images that do not
    //really have borders (single color background, broken jpgs...)
    static constexpr bool   ONLY_BLACK_BORDER = true;   //true: black border only. false: border color = outer edge pixels

    static constexpr int    SKIP_EDGE_PIXELS = 64;      //pick every nth pixel when checking edge for a border
    static constexpr double EDGE_BORDER_RATIO = 0.20;   //same color must be this much of edge for image to have a border
    static constexpr double MAX_BORDER_PERCENT = 0.95;  //up to 95% of image can be border
    static constexpr int    DEFAULT_DEVIATION = 30;     //don't seek more than 30px for edge
    static constexpr int    BORDER_THRESHOLD = 35;      //border/image color difference
    static constexpr int    NOT_A_BORDER = 4;           //differing pixels needed for row/col to not be border
};

#endif // PIC_H
