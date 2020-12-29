#ifndef SINGLEVIDEOSOURCE_H
#define SINGLEVIDEOSOURCE_H

#include <QtCore>

#include "video.h"
#include "videosource.h"

class SingleVideoSource : public VideoSource {
    Q_OBJECT

public:
    SingleVideoSource(QObject *parent = nullptr);

    void setVideo(Video *value);
    void setVideoId(const QString &value);

    void loadVideos(int max, int startIndex);
    bool hasMoreVideos();
    void abort() { aborted = true; }
    QString getName();
    const QList<QAction *> &getActions();
    int maxResults();

private:
    template <typename SourceType> SourceType setupSource(SourceType s) {
        if (video)
            s->setVideo(video);
        else
            s->setVideoId(videoId);
        return s;
    }
    void connectSource(int max, int startIndex);

    Video *video = nullptr;
    QString videoId;
    VideoSource *source = nullptr;
    bool aborted = false;
    QStringList emittedVideoIds;
};

#endif // SINGLEVIDEOSOURCE_H
