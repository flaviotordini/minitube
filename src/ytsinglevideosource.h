#ifndef YTSINGLEVIDEOSOURCE_H
#define YTSINGLEVIDEOSOURCE_H

#include <QtNetwork>
#include "videosource.h"

class YTSingleVideoSource : public VideoSource {

    Q_OBJECT

public:
    YTSingleVideoSource(QObject *parent = 0);
    void loadVideos(int max, int skip);
    void abort();
    const QStringList & getSuggestions();
    QString getName();

    void setVideoId(QString videoId) { this->videoId = videoId; }

private slots:
    void parse(QByteArray data);
    void requestError(QNetworkReply *reply);

private:
    QString videoId;
    bool aborted;
    int skip;
    int max;
};

#endif // YTSINGLEVIDEOSOURCE_H
