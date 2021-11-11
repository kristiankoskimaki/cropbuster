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

    QString filename;
    QSize size;
    QPoint origin = {0, 0};

signals:
    void add_this_image(Pic*add_me);
};

#endif // PIC_H
