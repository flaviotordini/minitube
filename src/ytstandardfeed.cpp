#include "ytstandardfeed.h"
#include <QtXml>
#include "networkaccess.h"
#include "video.h"
#include "ytfeedreader.h"

namespace The {
NetworkAccess* http();
}

YTStandardFeed::YTStandardFeed(QObject *parent)
    : VideoSource(parent),
      aborted(false) { }

void YTStandardFeed::loadVideos(int max, int skip) {
    aborted = false;

    QString s = "https://gdata.youtube.com/feeds/api/standardfeeds/";
    if (!regionId.isEmpty()) s += regionId + "/";
    s += feedId;
    if (!category.isEmpty()) s += "_" + category;

    QUrl url(s);
    url.addQueryItem("v", "2");

    if (feedId != "most_shared")
        url.addQueryItem("time", "today");

    url.addQueryItem("max-results", QString::number(max));
    url.addQueryItem("start-index", QString::number(skip));

    QObject *reply = The::http()->get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(parse(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), SLOT(requestError(QNetworkReply*)));
}

void YTStandardFeed::abort() {
    aborted = true;
}

const QStringList & YTStandardFeed::getSuggestions() {
    QStringList *l = new QStringList();
    return *l;
}

void YTStandardFeed::parse(QByteArray data) {
    if (aborted) return;

    YTFeedReader reader(data);
    QList<Video*> videos = reader.getVideos();

    foreach (Video *video, videos)
        emit gotVideo(video);

    emit finished(videos.size());
}

void YTStandardFeed::requestError(QNetworkReply *reply) {
    emit error(reply->errorString());
}
