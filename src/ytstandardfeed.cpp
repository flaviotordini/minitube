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

#include "ytstandardfeed.h"
#include "networkaccess.h"
#include "video.h"

#ifdef APP_YT3
#include "yt3.h"
#include "yt3listparser.h"
#else
#include "ytfeedreader.h"
#endif

namespace The {
NetworkAccess* http();
}

YTStandardFeed::YTStandardFeed(QObject *parent)
    : PaginatedVideoSource(parent),
      aborted(false) { }

#ifdef APP_YT3

void YTStandardFeed::loadVideos(int max, int startIndex) {
    aborted = false;

    QUrl url = YT3::instance().method("videos");

#if QT_VERSION >= 0x050000
    {
        QUrl &u = url;
        QUrlQuery url;
#endif

        if (startIndex > 1) {
            if (maybeReloadToken(max, startIndex)) return;
            url.addQueryItem("pageToken", nextPageToken);
        }

        url.addQueryItem("part", "snippet,contentDetails,statistics");
        url.addQueryItem("chart", "mostPopular");

        if (!category.isEmpty())
            url.addQueryItem("videoCategoryId", category);

        if (!regionId.isEmpty())
            url.addQueryItem("regionCode", regionId);

        url.addQueryItem("maxResults", QString::number(max));

#if QT_VERSION >= 0x050000
        u.setQuery(url);
    }
#endif
    QObject *reply = The::http()->get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(parseResults(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), SLOT(requestError(QNetworkReply*)));
}

void YTStandardFeed::parseResults(QByteArray data) {
    if (aborted) return;

    YT3ListParser parser(data);
    QList<Video*> videos = parser.getVideos();

    bool tryingWithNewToken = setPageToken(parser.getNextPageToken());
    if (tryingWithNewToken) return;

    if (reloadingToken) {
        reloadingToken = false;
        loadVideos(currentMax, currentStartIndex);
        currentMax = currentStartIndex = 0;
        return;
    }

    emit gotVideos(videos);
    emit finished(videos.size());
}

#else

void YTStandardFeed::loadVideos(int max, int startIndex) {
    aborted = false;

    QString s = "http://gdata.youtube.com/feeds/api/standardfeeds/";
    if (!regionId.isEmpty()) s += regionId + "/";
    s += feedId;
    if (!category.isEmpty()) s += "_" + category;

    QUrl url(s);
#if QT_VERSION >= 0x050000
    {
        QUrl &u = url;
        QUrlQuery url;
#endif
        url.addQueryItem("v", "2");

        if (feedId != "most_shared" && feedId != "on_the_web") {
            QString t = time;
            if (t.isEmpty()) t = "today";
            url.addQueryItem("time", t);
        }

        url.addQueryItem("max-results", QString::number(max));
        url.addQueryItem("start-index", QString::number(startIndex));

#if QT_VERSION >= 0x050000
        u.setQuery(url);
    }
#endif
    QObject *reply = The::http()->get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(parse(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), SLOT(requestError(QNetworkReply*)));
}

void YTStandardFeed::parseResults(QByteArray data) {
    if (aborted) return;

    YTFeedReader reader(data);
    QList<Video*> videos = reader.getVideos();

    emit gotVideos(videos);
    emit finished(videos.size());
}

#endif

void YTStandardFeed::abort() {
    aborted = true;
}

const QStringList & YTStandardFeed::getSuggestions() {
    QStringList *l = new QStringList();
    return *l;
}

void YTStandardFeed::requestError(QNetworkReply *reply) {
    emit error(reply->errorString());
}
