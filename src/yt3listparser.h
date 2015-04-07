#ifndef YT3LISTPARSER_H
#define YT3LISTPARSER_H

#include <QtCore>
#include <QtScript>

class Video;

class YT3ListParser : public QObject {

public:
    YT3ListParser(const QByteArray &bytes);
    const QList<Video*> &getVideos() { return videos; }
    const QStringList &getSuggestions() { return suggestions; }
    const QString &getNextPageToken() { return nextPageToken; }

private:
    void parseItem(const QScriptValue &item);

    QList<Video*> videos;
    QStringList suggestions;
    QString nextPageToken;
};

#endif // YT3LISTPARSER_H
