#ifndef YT3LISTPARSER_H
#define YT3LISTPARSER_H

#include <QtCore>

class Video;

class YT3ListParser : public QObject {
public:
    YT3ListParser(const QByteArray &bytes);
    const QVector<Video *> &getVideos() { return videos; }
    const QString &getNextPageToken() { return nextPageToken; }

private:
    void parseItem(const QJsonObject &item);

    QVector<Video *> videos;
    QString nextPageToken;
};

#endif // YT3LISTPARSER_H
