#ifndef YTJSSINGLEVIDEOSOURCE_H
#define YTJSSINGLEVIDEOSOURCE_H

#include <QtCore>

#include "videosource.h"

class YTJSSingleVideoSource : public VideoSource {
    Q_OBJECT

public:
    explicit YTJSSingleVideoSource(QObject *parent = 0);

    void setVideoId(const QString &value) { videoId = value; }
    void setVideo(Video *video);

    void loadVideos(int max, int startIndex);
    void abort();
    QString getName();

private:
    Video *video;
    QString videoId;
    bool aborted = false;
    QString name;
};

#endif // YTJSSINGLEVIDEOSOURCE_H
