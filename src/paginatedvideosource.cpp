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

#include "paginatedvideosource.h"

#include "yt3.h"
#include "yt3listparser.h"
#include "datautils.h"

#include "video.h"
#include "http.h"
#include "httputils.h"

PaginatedVideoSource::PaginatedVideoSource(QObject *parent) : VideoSource(parent)
  , tokenTimestamp(0)
  , reloadingToken(false)
  , currentMax(0)
  , currentStartIndex(0)
  , asyncDetails(false) { }

bool PaginatedVideoSource::hasMoreVideos() {
    qDebug() << __PRETTY_FUNCTION__ << nextPageToken;
    return !nextPageToken.isEmpty();
}

bool PaginatedVideoSource::maybeReloadToken(int max, int startIndex) {
    // kind of hackish. Thank the genius who came up with this stateful stuff
    // in a supposedly RESTful (aka stateless) API.

    if (nextPageToken.isEmpty()) {
        // previous request did not return a page token. Game over.
        // emit gotVideos(QList<Video*>());
        emit finished(0);
        return true;
    }

    if (isPageTokenExpired()) {
        reloadingToken = true;
        currentMax = max;
        currentStartIndex = startIndex;
        reloadToken();
        return true;
    }
    return false;
}

bool PaginatedVideoSource::setPageToken(const QString &value) {
    tokenTimestamp = QDateTime::currentDateTime().toTime_t();
    nextPageToken = value;

    if (reloadingToken) {
        reloadingToken = false;
        loadVideos(currentMax, currentStartIndex);
        currentMax = currentStartIndex = 0;
        return true;
    }

    return false;
}

bool PaginatedVideoSource::isPageTokenExpired() {
    uint now = QDateTime::currentDateTime().toTime_t();
    return now - tokenTimestamp > 1800;
}

void PaginatedVideoSource::reloadToken() {
    qDebug() << "Reloading pageToken";
    QObject *reply = HttpUtils::yt().get(lastUrl);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(parseResults(QByteArray)));
    connect(reply, SIGNAL(error(QString)), SLOT(requestError(QString)));
}

void PaginatedVideoSource::loadVideoDetails(const QList<Video*> &videos) {
    QString videoIds;
    foreach (Video *video, videos) {
        // TODO get video details from cache
        if (!videoIds.isEmpty()) videoIds += ",";
        videoIds += video->id();
        this->videos = videos;
        videoMap.insert(video->id(), video);
    }

    if (videoIds.isEmpty()) {
        if (!asyncDetails) {
            emit gotVideos(videos);
            emit finished(videos.size());
        }
        return;
    }

    QUrl url = YT3::instance().method("videos");
    QUrlQuery q(url);
    q.addQueryItem("part", "contentDetails,statistics");
    q.addQueryItem("id", videoIds);
    url.setQuery(q);

    QObject *reply = HttpUtils::yt().get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(parseVideoDetails(QByteArray)));
    connect(reply, SIGNAL(error(QString)), SLOT(requestError(QString)));
}

void PaginatedVideoSource::parseVideoDetails(const QByteArray &bytes) {
    QJsonDocument doc = QJsonDocument::fromJson(bytes);
    QJsonObject obj = doc.object();

    QJsonValue items = obj["items"];
    if (items.isArray()) {
        foreach (const QJsonValue &v, items.toArray()) {
            if (!v.isObject()) continue;

            QJsonObject item = v.toObject();

            QString id = item["id"].toString();
            Video *video = videoMap.value(id);
            if (!video) {
                qWarning() << "No video for id" << id;
                continue;
            }

            QString isoPeriod = item["contentDetails"].toObject()["duration"].toString();
            int duration = DataUtils::parseIsoPeriod(isoPeriod);
            video->setDuration(duration);

            int viewCount = item["statistics"].toObject()["viewCount"].toString().toInt();
            video->setViewCount(viewCount);

            // TODO cache by etag?
        }
    }
    if (!asyncDetails) {
        emit gotVideos(videos);
        emit finished(videos.size());
    } else {
        emit gotDetails();
    }
}
