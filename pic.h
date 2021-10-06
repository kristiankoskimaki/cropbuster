#ifndef PIC_H
#define PIC_H

#include <QString>
#include <QImage>
#include <QSize>
#include <QPoint>

class pic
{
public:
    pic();

    QString filename;
    QImage thumb;
    QSize size;
    QPoint origin = {0, 0};

private slots:
    void findFrame(const QString &filename);
};

#endif // PIC_H
