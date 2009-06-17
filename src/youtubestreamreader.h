#ifndef YOUTUBESTREAMREADER_H
#define YOUTUBESTREAMREADER_H

#include <QXmlStreamReader>
#include <QBuffer>
#include "video.h"

class YouTubeStreamReader : public QXmlStreamReader
{
public:
    YouTubeStreamReader();
    bool read(QByteArray data);
    QList<Video*> getVideos();

private:
    void readMediaGroup();
    void readEntry();
    QList<Video*> videos;
};

#endif // YOUTUBESTREAMREADER_H
