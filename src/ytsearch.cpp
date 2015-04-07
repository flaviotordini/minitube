/* $BEGIN_LICENSE

This file is part of Minitube.
Copyright 2009, Flavio Tordini <flavio.tordini@gmail.com>

Minitube is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Minitube is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Minitube.  If not, see <http://www.gnu.org/licenses/>.

$END_LICENSE */

#include "ytsearch.h"
#include "constants.h"
#include "networkaccess.h"
#include "searchparams.h"
#include "video.h"
#include "ytchannel.h"

#ifdef APP_YT3
#include "yt3.h"
#include "yt3listparser.h"
#include "datautils.h"
#else
#include "ytfeedreader.h"
#endif

namespace The {
NetworkAccess* http();
QHash<QString, QAction*>* globalActions();
}

namespace {

QDateTime RFC3339fromString(const QString &s) {
    return QDateTime::fromString(s, "yyyy-MM-ddThh:mm:ssZ");
}

QString RFC3339toString(const QDateTime &dt) {
    return dt.toString("yyyy-MM-ddThh:mm:ssZ");
}

}

YTSearch::YTSearch(SearchParams *searchParams, QObject *parent) :
    PaginatedVideoSource(parent),
    searchParams(searchParams) {
    searchParams->setParent(this);
}

#ifdef APP_YT3

void YTSearch::loadVideos(int max, int startIndex) {
    aborted = false;

    QUrl url = YT3::instance().method("search");

#if QT_VERSION >= 0x050000
    {
        QUrl &u = url;
        QUrlQuery url;
#endif

        url.addQueryItem("part", "snippet");
        url.addQueryItem("type", "video");

        url.addQueryItem("maxResults", QString::number(max));

        if (startIndex > 1) {
            if (maybeReloadToken(max, startIndex)) return;
            url.addQueryItem("pageToken", nextPageToken);
        }

        // TODO interesting params
        // url.addQueryItem("videoSyndicated", "true");
        // url.addQueryItem("regionCode", "IT");
        // url.addQueryItem("videoType", "movie");

        if (!searchParams->keywords().isEmpty()) {
            if (searchParams->keywords().startsWith("http://") ||
                    searchParams->keywords().startsWith("https://")) {
                url.addQueryItem("q", YTSearch::videoIdFromUrl(searchParams->keywords()));
            } else url.addQueryItem("q", searchParams->keywords());
        }

        if (!searchParams->channelId().isEmpty())
            url.addQueryItem("channelId", searchParams->channelId());

        switch (searchParams->sortBy()) {
        case SearchParams::SortByNewest:
            url.addQueryItem("order", "date");
            break;
        case SearchParams::SortByViewCount:
            url.addQueryItem("order", "viewCount");
            break;
        case SearchParams::SortByRating:
            url.addQueryItem("order", "rating");
            break;
        }

        switch (searchParams->duration()) {
        case SearchParams::DurationShort:
            url.addQueryItem("videoDuration", "short");
            break;
        case SearchParams::DurationMedium:
            url.addQueryItem("videoDuration", "medium");
            break;
        case SearchParams::DurationLong:
            url.addQueryItem("videoDuration", "long");
            break;
        }

        switch (searchParams->time()) {
        case SearchParams::TimeToday:
            url.addQueryItem("publishedAfter", RFC3339toString(QDateTime::currentDateTimeUtc().addSecs(-60*60*24)));
            break;
        case SearchParams::TimeWeek:
            url.addQueryItem("publishedAfter", RFC3339toString(QDateTime::currentDateTimeUtc().addSecs(-60*60*24*7)));
            break;
        case SearchParams::TimeMonth:
            url.addQueryItem("publishedAfter", RFC3339toString(QDateTime::currentDateTimeUtc().addSecs(-60*60*24*30)));
            break;
        }

        if (searchParams->publishedAfter()) {
            url.addQueryItem("publishedAfter", RFC3339toString(QDateTime::fromTime_t(searchParams->publishedAfter()).toUTC()));
        }

        switch (searchParams->quality()) {
        case SearchParams::QualityHD:
            url.addQueryItem("videoDefinition", "high");
            break;
        }

#if QT_VERSION >= 0x050000
        u.setQuery(url);
    }
#endif

    lastUrl = url;

    // qWarning() << "YT3 search" << url.toString();
    QObject *reply = The::http()->get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(parseResults(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), SLOT(requestError(QNetworkReply*)));
}

void YTSearch::parseResults(QByteArray data) {
    if (aborted) return;

    YT3ListParser parser(data);
    QList<Video*> videos = parser.getVideos();
    suggestions = parser.getSuggestions();

    bool tryingWithNewToken = setPageToken(parser.getNextPageToken());
    if (tryingWithNewToken) return;

    if (name.isEmpty() && !searchParams->channelId().isEmpty()) {
        if (!videos.isEmpty()) {
            name = videos.first()->channelTitle();
        }
        emit nameChanged(name);
    }

    if (asyncDetails) {
        emit gotVideos(videos);
        emit finished(videos.size());
    }
    loadVideoDetails(videos);
}

#else

void YTSearch::loadVideos(int max, int startIndex) {
    aborted = false;

    QUrl url("http://gdata.youtube.com/feeds/api/videos/");
#if QT_VERSION >= 0x050000
    {
        QUrl &u = url;
        QUrlQuery url;
#endif

        url.addQueryItem("v", "2");
        url.addQueryItem("max-results", QString::number(max));
        url.addQueryItem("start-index", QString::number(startIndex));

        if (!searchParams->keywords().isEmpty()) {
            if (searchParams->keywords().startsWith("http://") ||
                    searchParams->keywords().startsWith("https://")) {
                url.addQueryItem("q", YTSearch::videoIdFromUrl(searchParams->keywords()));
            } else url.addQueryItem("q", searchParams->keywords());
        }

        if (!searchParams->channelId().isEmpty())
            url.addQueryItem("author", searchParams->channelId());

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

#if QT_VERSION >= 0x050000
        u.setQuery(url);
    }
#endif
    QObject *reply = The::http()->get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(parseResults(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), SLOT(requestError(QNetworkReply*)));
}

void YTSearch::parseResults(QByteArray data) {
    if (aborted) return;

    YTFeedReader reader(data);
    QList<Video*> videos = reader.getVideos();
    suggestions = reader.getSuggestions();

    if (name.isEmpty() && !searchParams->channelId().isEmpty()) {
        if (videos.isEmpty()) name = searchParams->channelId();
        else {
            name = videos.first()->channelTitle();
            // also grab the userId
            userId = videos.first()->channelId();
        }
        emit nameChanged(name);
    }

    emit gotVideos(videos);
    emit finished(videos.size());
}

#endif

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

void YTSearch::requestError(QNetworkReply *reply) {
    qWarning() << reply->errorString();
    emit error(reply->errorString());
}

QString YTSearch::videoIdFromUrl(QString url) {
    QRegExp re = QRegExp("^.*[\\?&]v=([^&#]+).*$");
    if (re.exactMatch(url)) return re.cap(1);
    re = QRegExp("^.*://.*/([^&#\\?]+).*$");
    if (re.exactMatch(url)) return re.cap(1);
    return QString();
}

QList<QAction*> YTSearch::getActions() {
    QList<QAction*> channelActions;
    if (searchParams->channelId().isEmpty())
        return channelActions;
    channelActions << The::globalActions()->value("subscribe-channel");
    return channelActions;
}
