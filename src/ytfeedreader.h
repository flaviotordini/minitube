#ifndef YTFEEDREADER_H
#define YTFEEDREADER_H

#include <QtXml>

class Video;

class YTFeedReader : public QXmlStreamReader {

public:
    YTFeedReader(const QByteArray &bytes);
    const QList<Video*> & getVideos();
    const QStringList & getSuggestions() const;

private:
    void readEntry();
    QList<Video*> videos;
    QStringList suggestions;
};

#endif // YTFEEDREADER_H
