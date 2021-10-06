#ifndef PIC_H
#define PIC_H

#include <QString>
#include <QImage>
#include <QSize>
#include <QPoint>
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>

class Pic
{
public:
    Pic(const QString &filenameParam);
    void run();

    QString filename;
    QImage thumb;
    QSize size;
    QPoint origin = {0, 0};

private slots:
    void findFrame();
};

#endif // PIC_H
