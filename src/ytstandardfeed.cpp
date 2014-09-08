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
    url.addQueryItem("start-index", QString::number(skip));

#if QT_VERSION >= 0x050000
        u.setQuery(url);
    }
#endif
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

    emit gotVideos(videos);
    emit finished(videos.size());
}

void YTStandardFeed::requestError(QNetworkReply *reply) {
    emit error(reply->errorString());
}
