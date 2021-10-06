#include "pic.h"

Pic::Pic(const QString &filenameParam) : filename(filenameParam)
{
}

void Pic::run()
{
    if(!QFileInfo::exists(filename))
        return;
    findFrame();
}

void Pic::findFrame()
{
}
