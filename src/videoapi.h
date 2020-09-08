#ifndef VIDEOAPI_H
#define VIDEOAPI_H

#include <QtCore>

class VideoAPI {
public:
    enum Impl { YT3, IV, JS };
    static Impl impl() { return JS; }

private:
    VideoAPI() {}
};

#endif // VIDEOAPI_H
