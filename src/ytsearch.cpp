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
#include "http.h"
#include "httputils.h"
#include "searchparams.h"
#include "video.h"
#include "ytchannel.h"

#include "yt3.h"
#include "yt3listparser.h"
#include "datautils.h"
#include "mainwindow.h"

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

void YTSearch::loadVideos(int max, int startIndex) {
    aborted = false;

    QUrl url = YT3::instance().method("search");

    QUrlQuery q(url);
    q.addQueryItem("part", "snippet");
    q.addQueryItem("type", "video");
    q.addQueryItem("maxResults", QString::number(max));

    if (startIndex > 1) {
        if (maybeReloadToken(max, startIndex)) return;
        q.addQueryItem("pageToken", nextPageToken);
    }

    // TODO interesting params
    // urlHelper.addQueryItem("videoSyndicated", "true");
    // urlHelper.addQueryItem("regionCode", "IT");
    // urlHelper.addQueryItem("videoType", "movie");

    if (!searchParams->keywords().isEmpty()) {
        if (searchParams->keywords().startsWith("http://") ||
                searchParams->keywords().startsWith("https://")) {
            q.addQueryItem("q", YTSearch::videoIdFromUrl(searchParams->keywords()));
        } else q.addQueryItem("q", searchParams->keywords());
    }

    if (!searchParams->channelId().isEmpty())
        q.addQueryItem("channelId", searchParams->channelId());

    switch (searchParams->sortBy()) {
    case SearchParams::SortByNewest:
        q.addQueryItem("order", "date");
        break;
    case SearchParams::SortByViewCount:
        q.addQueryItem("order", "viewCount");
        break;
    case SearchParams::SortByRating:
        q.addQueryItem("order", "rating");
        break;
    }

    switch (searchParams->duration()) {
    case SearchParams::DurationShort:
        q.addQueryItem("videoDuration", "short");
        break;
    case SearchParams::DurationMedium:
        q.addQueryItem("videoDuration", "medium");
        break;
    case SearchParams::DurationLong:
        q.addQueryItem("videoDuration", "long");
        break;
    }

    switch (searchParams->time()) {
    case SearchParams::TimeToday:
        q.addQueryItem("publishedAfter", RFC3339toString(QDateTime::currentDateTimeUtc().addSecs(-60*60*24)));
        break;
    case SearchParams::TimeWeek:
        q.addQueryItem("publishedAfter", RFC3339toString(QDateTime::currentDateTimeUtc().addSecs(-60*60*24*7)));
        break;
    case SearchParams::TimeMonth:
        q.addQueryItem("publishedAfter", RFC3339toString(QDateTime::currentDateTimeUtc().addSecs(-60*60*24*30)));
        break;
    }

    if (searchParams->publishedAfter()) {
        q.addQueryItem("publishedAfter", RFC3339toString(QDateTime::fromTime_t(searchParams->publishedAfter()).toUTC()));
    }

    switch (searchParams->quality()) {
    case SearchParams::QualityHD:
        q.addQueryItem("videoDefinition", "high");
        break;
    }

    switch (searchParams->safeSearch()) {
    case SearchParams::None:
        q.addQueryItem("safeSearch", "none");
        break;
    case SearchParams::Strict:
        q.addQueryItem("safeSearch", "strict");
        break;
    }

    url.setQuery(q);

    lastUrl = url;

    // qWarning() << "YT3 search" << url.toString();
    QObject *reply = HttpUtils::yt().get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(parseResults(QByteArray)));
    connect(reply, SIGNAL(error(QString)), SLOT(requestError(QString)));
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

void YTSearch::requestError(const QString &message) {
    qWarning() << message;
    emit error(message);
}

QString YTSearch::videoIdFromUrl(const QString &url) {
    QRegExp re = QRegExp("^.*[\\?&]v=([^&#]+).*$");
    if (re.exactMatch(url)) return re.cap(1);
    re = QRegExp("^.*://.*/([^&#\\?]+).*$");
    if (re.exactMatch(url)) return re.cap(1);
    return QString();
}

QTime YTSearch::videoTimestampFromUrl(const QString &url) {
    QTime res(0, 0);

    // TODO: should we make this accept h/m/s in any order?
    //       timestamps returned by youtube always seem to be
    //       ordered.
    QRegExp re = QRegExp(".*t=([0-9]*h)?([0-9]*m)?([0-9]*s)?.*");

    if (!re.exactMatch(url)) {
        return res;
    }

    foreach (const QString &str, re.capturedTexts()) {
        if (str.length() <= 1) continue;

        QString truncated = str;
        truncated.chop(1);

        bool ok = false;
        int value = truncated.toInt(&ok);
        if (!ok) continue;
        char unit = (*str.rbegin()).toLatin1();

        switch (unit)
        {
            case 'h':
                value *= 60 * 60; // hours -> seconds
                break;

            case 'm':
                value *= 60; // minutes -> seconds
                break;

            case 's':
                break;

            default:
                continue;
        }

        res = res.addSecs(value);
    }

    return res;
}

QList<QAction*> YTSearch::getActions() {
    QList<QAction*> channelActions;
    if (searchParams->channelId().isEmpty())
        return channelActions;
    channelActions << MainWindow::instance()->getActionMap().value("subscribe-channel");
    return channelActions;
}
