#include "pic.h"

Pic::Pic(QObject *_mainwPtr, const QString &filenameParam) : filename(filenameParam)
{
    QObject::connect(this, SIGNAL(add_this_image(Pic*)), _mainwPtr, SLOT(add_image_with_borders(Pic*)));
}

void Pic::run()
{
    if(!QFileInfo::exists(filename))
        return;
    findFrame();
}

void Pic::findFrame()
{
    emit add_this_image(this);
}
