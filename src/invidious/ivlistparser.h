#ifndef IVLISTPARSER_H
#define IVLISTPARSER_H

#include <QtCore>

class Video;

class IVListParser {
public:
    IVListParser(const QJsonArray &items);
    const QVector<Video *> &getVideos() { return videos; }

private:
    void parseItem(const QJsonObject &item);

    QVector<Video *> videos;
};

#endif // IVLISTPARSER_H
