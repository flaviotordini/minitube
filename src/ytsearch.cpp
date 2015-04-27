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
#include "compatibility/qurlqueryhelper.h"

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
    {
        QUrlQueryHelper urlHelper(url);
        urlHelper.addQueryItem("part", "snippet");
        urlHelper.addQueryItem("type", "video");
        urlHelper.addQueryItem("maxResults", QString::number(max));

        if (startIndex > 1) {
            if (maybeReloadToken(max, startIndex)) return;
            urlHelper.addQueryItem("pageToken", nextPageToken);
        }

        // TODO interesting params
        // urlHelper.addQueryItem("videoSyndicated", "true");
        // urlHelper.addQueryItem("regionCode", "IT");
        // urlHelper.addQueryItem("videoType", "movie");

        if (!searchParams->keywords().isEmpty()) {
            if (searchParams->keywords().startsWith("http://") ||
                    searchParams->keywords().startsWith("https://")) {
                urlHelper.addQueryItem("q", YTSearch::videoIdFromUrl(searchParams->keywords()));
            } else urlHelper.addQueryItem("q", searchParams->keywords());
        }

        if (!searchParams->channelId().isEmpty())
            urlHelper.addQueryItem("channelId", searchParams->channelId());

        switch (searchParams->sortBy()) {
        case SearchParams::SortByNewest:
            urlHelper.addQueryItem("order", "date");
            break;
        case SearchParams::SortByViewCount:
            urlHelper.addQueryItem("order", "viewCount");
            break;
        case SearchParams::SortByRating:
            urlHelper.addQueryItem("order", "rating");
            break;
        }

        switch (searchParams->duration()) {
        case SearchParams::DurationShort:
            urlHelper.addQueryItem("videoDuration", "short");
            break;
        case SearchParams::DurationMedium:
            urlHelper.addQueryItem("videoDuration", "medium");
            break;
        case SearchParams::DurationLong:
            urlHelper.addQueryItem("videoDuration", "long");
            break;
        }

        switch (searchParams->time()) {
        case SearchParams::TimeToday:
            urlHelper.addQueryItem("publishedAfter", RFC3339toString(QDateTime::currentDateTimeUtc().addSecs(-60*60*24)));
            break;
        case SearchParams::TimeWeek:
            urlHelper.addQueryItem("publishedAfter", RFC3339toString(QDateTime::currentDateTimeUtc().addSecs(-60*60*24*7)));
            break;
        case SearchParams::TimeMonth:
            urlHelper.addQueryItem("publishedAfter", RFC3339toString(QDateTime::currentDateTimeUtc().addSecs(-60*60*24*30)));
            break;
        }

        if (searchParams->publishedAfter()) {
            urlHelper.addQueryItem("publishedAfter", RFC3339toString(QDateTime::fromTime_t(searchParams->publishedAfter()).toUTC()));
        }

        switch (searchParams->quality()) {
        case SearchParams::QualityHD:
            urlHelper.addQueryItem("videoDefinition", "high");
            break;
        }
    }

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
    {
        QUrlQueryHelper urlHelper(url);

        urlHelper.addQueryItem("v", "2");
        urlHelper.addQueryItem("max-results", QString::number(max));
        urlHelper.addQueryItem("start-index", QString::number(startIndex));

        if (!searchParams->keywords().isEmpty()) {
            if (searchParams->keywords().startsWith("http://") ||
                    searchParams->keywords().startsWith("https://")) {
                urlHelper.addQueryItem("q", YTSearch::videoIdFromUrl(searchParams->keywords()));
            } else urlHelper.addQueryItem("q", searchParams->keywords());
        }

        if (!searchParams->channelId().isEmpty())
            urlHelper.addQueryItem("author", searchParams->channelId());

        switch (searchParams->sortBy()) {
        case SearchParams::SortByNewest:
            urlHelper.addQueryItem("orderby", "published");
            break;
        case SearchParams::SortByViewCount:
            urlHelper.addQueryItem("orderby", "viewCount");
            break;
        case SearchParams::SortByRating:
            urlHelper.addQueryItem("orderby", "rating");
            break;
        }

        switch (searchParams->duration()) {
        case SearchParams::DurationShort:
            urlHelper.addQueryItem("duration", "short");
            break;
        case SearchParams::DurationMedium:
            urlHelper.addQueryItem("duration", "medium");
            break;
        case SearchParams::DurationLong:
            urlHelper.addQueryItem("duration", "long");
            break;
        }

        switch (searchParams->time()) {
        case SearchParams::TimeToday:
            urlHelper.addQueryItem("time", "today");
            break;
        case SearchParams::TimeWeek:
            urlHelper.addQueryItem("time", "this_week");
            break;
        case SearchParams::TimeMonth:
            urlHelper.addQueryItem("time", "this_month");
            break;
        }

        switch (searchParams->quality()) {
        case SearchParams::QualityHD:
            urlHelper.addQueryItem("hd", "true");
            break;
        }

    }
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
