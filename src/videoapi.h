#ifndef VIDEOAPI_H
#define VIDEOAPI_H

#include <QtCore>

class VideoAPI {
public:
    enum Impl { YT3, IV };
    static Impl impl() { return IV; }

private:
    VideoAPI() {}
};

#endif // VIDEOAPI_H
