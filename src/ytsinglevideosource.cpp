#include "ytsinglevideosource.h"
#include <QtXml>
#include "networkaccess.h"
#include "video.h"
#include "ytfeedreader.h"

namespace The {
NetworkAccess* http();
}

YTSingleVideoSource::YTSingleVideoSource(QObject *parent) : VideoSource(parent) {
    skip = 0;
    max = 0;
}

void YTSingleVideoSource::loadVideos(int max, int skip) {
    aborted = false;
    this->skip = skip;
    this->max = max;

    QString s;
    if (skip == 1) s = "http://gdata.youtube.com/feeds/api/videos/" + videoId;
    else s = QString("http://gdata.youtube.com/feeds/api/videos/%1/related").arg(videoId);
    QUrl url(s);
    url.addQueryItem("v", "2");

    if (skip != 1) {
        url.addQueryItem("max-results", QString::number(max));
        url.addQueryItem("start-index", QString::number(skip-1));
    }

    QObject *reply = The::http()->get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(parse(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), SLOT(requestError(QNetworkReply*)));
}

void YTSingleVideoSource::abort() {
    aborted = true;
}

const QStringList & YTSingleVideoSource::getSuggestions() {
    QStringList *l = new QStringList();
    return *l;
}

QString YTSingleVideoSource::getName() {
    return name;
}

void YTSingleVideoSource::parse(QByteArray data) {
    if (aborted) return;

    YTFeedReader reader(data);
    QList<Video*> videos = reader.getVideos();

    if (name.isEmpty() && !videos.isEmpty() && skip == 1) {
        name = videos.first()->title();
        emit nameChanged(name);
    }

    emit gotVideos(videos);

    if (skip == 1) loadVideos(max - 1, 2);
    else if (skip == 2) emit finished(videos.size() + 1);
    else emit finished(videos.size());
}

void YTSingleVideoSource::requestError(QNetworkReply *reply) {
    emit error(reply->errorString());
}
