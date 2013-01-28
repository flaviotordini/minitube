#include "ytsearch.h"
#include "ytfeedreader.h"
#include "constants.h"
#include "networkaccess.h"
#include "searchparams.h"
#include "video.h"

namespace The {
    NetworkAccess* http();
}

YTSearch::YTSearch(SearchParams *searchParams, QObject *parent) :
    VideoSource(parent),
    searchParams(searchParams) {
    searchParams->setParent(this);
}

void YTSearch::loadVideos(int max, int skip) {
    aborted = false;

    QUrl url("http://gdata.youtube.com/feeds/api/videos/");
    url.addQueryItem("v", "2");

    url.addQueryItem("max-results", QString::number(max));
    url.addQueryItem("start-index", QString::number(skip));

    if (!searchParams->keywords().isEmpty()) {
        if (searchParams->keywords().startsWith("http://") ||
                searchParams->keywords().startsWith("https://")) {
            url.addQueryItem("q", YTSearch::videoIdFromUrl(searchParams->keywords()));
        } else url.addQueryItem("q", searchParams->keywords());
    }

    if (!searchParams->author().isEmpty())
        url.addQueryItem("author", searchParams->author());

    switch (searchParams->sortBy()) {
    case SearchParams::SortByNewest:
        url.addQueryItem("orderby", "published");
        break;
    case SearchParams::SortByViewCount:
        url.addQueryItem("orderby", "viewCount");
        break;
    case SearchParams::SortByRating:
        url.addQueryItem("orderby", "rating");
        break;
    }

    switch (searchParams->duration()) {
    case SearchParams::DurationShort:
        url.addQueryItem("duration", "short");
        break;
    case SearchParams::DurationMedium:
        url.addQueryItem("duration", "medium");
        break;
    case SearchParams::DurationLong:
        url.addQueryItem("duration", "long");
        break;
    }

    switch (searchParams->time()) {
    case SearchParams::TimeToday:
        url.addQueryItem("time", "today");
        break;
    case SearchParams::TimeWeek:
        url.addQueryItem("time", "this_week");
        break;
    case SearchParams::TimeMonth:
        url.addQueryItem("time", "this_month");
        break;
    }

    switch (searchParams->quality()) {
    case SearchParams::QualityHD:
        url.addQueryItem("hd", "true");
        break;
    }

    QObject *reply = The::http()->get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(parseResults(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), SLOT(requestError(QNetworkReply*)));
}

void YTSearch::abort() {
    aborted = true;
}

const QStringList & YTSearch::getSuggestions() {
    return suggestions;
}

QString YTSearch::getName() {
    if (!name.isEmpty()) return name;
    if (!searchParams->keywords().isEmpty()) return searchParams->keywords();
    return QString();
}

void YTSearch::parseResults(QByteArray data) {
    if (aborted) return;

    YTFeedReader reader(data);
    QList<Video*> videos = reader.getVideos();
    suggestions = reader.getSuggestions();

    if (name.isEmpty() && !searchParams->author().isEmpty()) {
        if (videos.isEmpty()) name = searchParams->author();
        else name = videos.first()->author();
        emit nameChanged(name);
    }

    emit gotVideos(videos);
    emit finished(videos.size());
}

void YTSearch::requestError(QNetworkReply *reply) {
    emit error(reply->errorString());
}

QString YTSearch::videoIdFromUrl(QString url) {
    QRegExp re = QRegExp("^.*[\\?&]v=([^&#]+).*$");
    if (re.exactMatch(url)) return re.cap(1);
    re = QRegExp("^.*://.*/([^&#\\?]+).*$");
    if (re.exactMatch(url)) return re.cap(1);
    return QString();
}
