#ifndef PIC_H
#define PIC_H

#include <opencv2/opencv.hpp>
#include <QRunnable>
#include <QImage>
#include <QDirIterator>

class Pic: public QObject, public QRunnable
{
    Q_OBJECT

public:
    Pic(QObject *_mainwPtr, const QString &filenameParam);
    void run();

    QString filename;
    QImage thumb;
    QSize size;
    QPoint origin = {0, 0};

private slots:
    bool findFrame();

signals:
    void add_this_image(Pic*add_me);
};

#endif // PIC_H
