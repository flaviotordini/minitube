#ifndef YT3LISTPARSER_H
#define YT3LISTPARSER_H

#include <QtCore>

class Video;

class YT3ListParser : public QObject {

public:
    YT3ListParser(const QByteArray &bytes);
    const QVector<Video*> &getVideos() { return videos; }
    const QStringList &getSuggestions() { return suggestions; }
    const QString &getNextPageToken() { return nextPageToken; }

private:
    void parseItem(const QJsonObject &item);

    QVector<Video*> videos;
    QStringList suggestions;
    QString nextPageToken;
};

#endif // YT3LISTPARSER_H
