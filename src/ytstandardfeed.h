#ifndef YTSTANDARDFEED_H
#define YTSTANDARDFEED_H

#include <QtNetwork>
#include "videosource.h"

class YTStandardFeed : public VideoSource {

    Q_OBJECT

public:
    YTStandardFeed(QObject *parent = 0);

    QString getFeedId() { return feedId; }
    void setFeedId(QString feedId) { this->feedId = feedId; }

    QString getRegionId() { return regionId; }
    void setRegionId(QString regionId) { this->regionId = regionId; }

    QString getCategory() { return category; }
    void setCategory(QString category) { this->category = category; }

    QString getLabel() { return label; }
    void setLabel(QString label) { this->label = label; }

    void loadVideos(int max, int skip);
    void abort();
    const QStringList & getSuggestions();
    QString getName() { return label; }

private slots:
    void parse(QByteArray data);
    void requestError(QNetworkReply *reply);

private:
    QString feedId;
    QString regionId;
    QString category;
    QString label;
    bool aborted;
};

#endif // YTSTANDARDFEED_H
