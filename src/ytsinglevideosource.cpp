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

#include "ytsinglevideosource.h"
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

YTSingleVideoSource::YTSingleVideoSource(QObject *parent) : PaginatedVideoSource(parent),
    video(0),
    startIndex(0),
    max(0) { }

#ifdef APP_YT3

void YTSingleVideoSource::loadVideos(int max, int startIndex) {
    aborted = false;
    this->startIndex = startIndex;
    this->max = max;

    QUrl url;

    if (startIndex == 1) {

        if (video) {
            QList<Video*> videos;
            videos << video->clone();
            if (name.isEmpty()) {
                name = videos.first()->title();
                qDebug() << "Emitting name changed" << name;
                emit nameChanged(name);
            }
            emit gotVideos(videos);
            loadVideos(max - 1, 2);
            return;
        }

        url = YT3::instance().method("videos");
        QUrlQuery q(url);
        q.addQueryItem("part", "snippet");
        q.addQueryItem("id", videoId);
        url.setQuery(q);

    } else {
        url = YT3::instance().method("search");
        QUrlQuery q(url);
        q.addQueryItem("part", "snippet");
        q.addQueryItem("type", "video");
        q.addQueryItem("relatedToVideoId", videoId);
        q.addQueryItem("maxResults", QString::number(max));
        if (startIndex > 2) {
            if (maybeReloadToken(max, startIndex)) return;
            q.addQueryItem("pageToken", nextPageToken);
        }
        url.setQuery(q);
    }

    lastUrl = url;

    QObject *reply = The::http()->get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(parseResults(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), SLOT(requestError(QNetworkReply*)));
}

void YTSingleVideoSource::parseResults(QByteArray data) {
    if (aborted) return;

    YT3ListParser parser(data);
    QList<Video*> videos = parser.getVideos();

    bool tryingWithNewToken = setPageToken(parser.getNextPageToken());
    if (tryingWithNewToken) return;

    if (asyncDetails) {
        emit gotVideos(videos);
        if (startIndex == 2) emit finished(videos.size() + 1);
        else emit finished(videos.size());
    }
    loadVideoDetails(videos);
}

#else

void YTSingleVideoSource::loadVideos(int max, int startIndex) {
    aborted = false;
    this->startIndex = startIndex;
    this->max = max;

    QString s;
    if (startIndex == 1) s = "http://gdata.youtube.com/feeds/api/videos/" + videoId;
    else s = QString("http://gdata.youtube.com/feeds/api/videos/%1/related").arg(videoId);
    QUrl url(s);
    {
        QUrlQueryHelper urlHelper(url);
        urlHelper.addQueryItem("v", "2");

        if (startIndex != 1) {
            urlHelper.addQueryItem("max-results", QString::number(max));
            urlHelper.addQueryItem("start-index", QString::number(startIndex-1));
        }
    }
    QObject *reply = The::http()->get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(parse(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), SLOT(requestError(QNetworkReply*)));
}

void YTSingleVideoSource::parse(QByteArray data) {
    if (aborted) return;

    YTFeedReader reader(data);
    QList<Video*> videos = reader.getVideos();

    if (name.isEmpty() && !videos.isEmpty() && startIndex == 1) {
        name = videos.first()->title();
        emit nameChanged(name);
    }

    emit gotVideos(videos);

    if (startIndex == 1) loadVideos(max - 1, 2);
    else if (startIndex == 2) emit finished(videos.size() + 1);
    else emit finished(videos.size());
}

#endif

void YTSingleVideoSource::abort() {
    aborted = true;
}

const QStringList & YTSingleVideoSource::getSuggestions() {
    static const QStringList l;
    return l;
}

QString YTSingleVideoSource::getName() {
    return name;
}

void YTSingleVideoSource::setVideo(Video *video) {
    this->video = video;
    videoId = video->id();
}

void YTSingleVideoSource::requestError(QNetworkReply *reply) {
    emit error(reply->errorString());
}
