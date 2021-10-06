#ifndef PIC_H
#define PIC_H

#include <QString>
#include <QImage>
#include <QSize>
#include <QPoint>
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>

class Pic: public QObject
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
    void findFrame();

signals:
    void add_this_image(Pic*add_me);
};

#endif // PIC_H
