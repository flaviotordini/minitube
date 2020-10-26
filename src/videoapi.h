#ifndef VIDEOAPI_H
#define VIDEOAPI_H

#include <QtCore>

#include "searchparams.h"
#include "videosource.h"

class VideoAPI {
public:
    enum Impl { YT3, IV, JS };
    static Impl impl() { return JS; }

private:
    VideoAPI() {}
};

#endif // VIDEOAPI_H
