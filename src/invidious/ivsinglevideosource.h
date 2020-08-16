#ifndef IVSINGLEVIDEOSOURCE_H
#define IVSINGLEVIDEOSOURCE_H

#include <QtCore>

#include "ivvideosource.h"

class IVSingleVideoSource : public IVVideoSource {
    Q_OBJECT
public:
    IVSingleVideoSource(QObject *parent = 0);

    void reallyLoadVideos(int max, int startIndex);
    QString getName();

    void setVideoId(const QString &value) { videoId = value; }
    void setVideo(Video *video);

private slots:
    void parseResults(QByteArray data);

private:
    Video *video;
    QString videoId;
    int startIndex;
    int max;
    QString name;
};

#endif // IVSINGLEVIDEOSOURCE_H
