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

#include "aggregatevideosource.h"
#include "video.h"
#include "database.h"
#include <QtSql>

AggregateVideoSource::AggregateVideoSource(QObject *parent) :
    VideoSource(parent),
    unwatched(false), hasMore(true) { }

void AggregateVideoSource::loadVideos(int max, int startIndex) {
    QSqlDatabase db = Database::instance().getConnection();
    QSqlQuery query(db);
    QString sql = "select v.video_id,"
            "v.published,"
            "v.title,"
            "v.author,"
            "v.user_id,"
            "v.description,"
            "v.url,"
            "v.thumb_url,"
            "v.views,"
            "v.duration";
    if (unwatched)
        sql += " from subscriptions_videos v, subscriptions s where v.channel_id=s.id "
                "and v.added>s.watched and v.published>s.watched and v.watched=0 "
                "order by v.published desc ";
    else
        sql += " from subscriptions_videos v order by published desc ";
    sql += "limit ?,?";
    query.prepare(sql);
    query.bindValue(0, startIndex - 1);
    query.bindValue(1, max);
    bool success = query.exec();
    if (!success) qWarning() << query.lastQuery() << query.lastError().text();
    QVector<Video*> videos;
    videos.reserve(query.size());
    while (query.next()) {
        Video *video = new Video();
        video->setId(query.value(0).toString());
        video->setPublished(QDateTime::fromSecsSinceEpoch(query.value(1).toUInt()));
        video->setTitle(query.value(2).toString());
        video->setChannelTitle(query.value(3).toString());
        video->setChannelId(query.value(4).toString());
        video->setDescription(query.value(5).toString());
        video->setWebpage(query.value(6).toString());

        QString thumbString = query.value(7).toString();
        if (thumbString.startsWith('[')) {
            const auto thumbs = QJsonDocument::fromJson(thumbString.toUtf8()).array();
            for (const auto &v : thumbs) {
                auto t = v.toObject();
                video->addThumb(t["width"].toInt(), t["height"].toInt(), t["url"].toString());
            }
        } else {
            // assume it's a URL
            video->addThumb(0, 0, thumbString);
        }

        video->setViewCount(query.value(8).toInt());
        video->setDuration(query.value(9).toInt());
        videos << video;
    }

    hasMore = videos.size() >= max;

    emit gotVideos(videos);
    emit finished(videos.size());
}

bool AggregateVideoSource::hasMoreVideos() {
    return hasMore;
}

void AggregateVideoSource::abort() { }
