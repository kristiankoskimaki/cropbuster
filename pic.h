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
    Pic(QObject *_mainwPtr, const QString &filenameParam);
    void run();
    void find_exact_edges(cv::Rect &rectangle);

    QString filename;
    QSize size;
    QPoint origin = {0, 0};

signals:
    void add_this_image(Pic*add_me);

private:
    static constexpr double MAX_BORDER_PERCENT = 0.95;  //up to 95% of image can be border
    static constexpr int    DEFAULT_DEVIATION = 30;     //don't seek more than 30px for edge
    static constexpr int    BORDER_THRESHOLD = 35;      //border/image color difference
};

#endif // PIC_H
