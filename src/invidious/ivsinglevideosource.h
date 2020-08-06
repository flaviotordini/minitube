#ifndef IVSINGLEVIDEOSOURCE_H
#define IVSINGLEVIDEOSOURCE_H

#include <QtCore>

#include "videosource.h"

class IVSingleVideoSource : public VideoSource {
    Q_OBJECT
public:
    IVSingleVideoSource(QObject *parent = 0);

    void loadVideos(int max, int startIndex);
    void abort();
    QString getName();

    void setVideoId(const QString &value) { videoId = value; }
    void setVideo(Video *video);

private slots:
    void parseResults(QByteArray data);
    void requestError(const QString &message);

private:
    Video *video;
    QString videoId;
    bool aborted;
    int startIndex;
    int max;
    QString name;
};

#endif // IVSINGLEVIDEOSOURCE_H
