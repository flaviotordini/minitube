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
#if QT_VERSION >= 0x050000
{
    QUrl &u = url;
    QUrlQuery url;
#endif
    url.addQueryItem("v", "2");

    if (skip != 1) {
        url.addQueryItem("max-results", QString::number(max));
        url.addQueryItem("start-index", QString::number(skip-1));
    }

#if QT_VERSION >= 0x050000
        u.setQuery(url);
    }
#endif
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
