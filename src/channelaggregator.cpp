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

#include "channelaggregator.h"
#include "ytchannel.h"
#include "ytsearch.h"
#include "searchparams.h"
#include "database.h"
#include "video.h"
#ifdef APP_MAC
#include "macutils.h"
#endif

ChannelAggregator::ChannelAggregator(QObject *parent) : QObject(parent),
    unwatchedCount(-1),
    running(false),
    stopped(false) {
    checkInterval = 3600;

    timer = new QTimer(this);
    timer->setInterval(60000 * 5);
    connect(timer, SIGNAL(timeout()), SLOT(run()));
}

ChannelAggregator* ChannelAggregator::instance() {
    static ChannelAggregator* i = new ChannelAggregator();
    return i;
}

void ChannelAggregator::start() {
    stopped = false;
    updateUnwatchedCount();
    QTimer::singleShot(0, this, SLOT(run()));
    if (!timer->isActive()) timer->start();
}

void ChannelAggregator::stop() {
    timer->stop();
    stopped = true;
}

YTChannel* ChannelAggregator::getChannelToCheck() {
    if (stopped) return 0;
    QSqlDatabase db = Database::instance().getConnection();
    QSqlQuery query(db);
    query.prepare("select user_id from subscriptions where checked<? "
                  "order by checked limit 1");
    query.bindValue(0, QDateTime::currentDateTimeUtc().toTime_t() - checkInterval);
    bool success = query.exec();
    if (!success) qWarning() << query.lastQuery() << query.lastError().text();
    if (query.next())
        return YTChannel::forId(query.value(0).toString());
    return 0;
}

void ChannelAggregator::run() {
    if (running) return;
    if (!Database::exists()) return;
    running = true;
    newVideoCount = 0;
    updatedChannels.clear();

    if (!Database::instance().getConnection().transaction())
        qWarning() << "Transaction failed" << __PRETTY_FUNCTION__;

    processNextChannel();
}

void ChannelAggregator::processNextChannel() {
    if (stopped) {
        running = false;
        return;
    }
    qApp->processEvents();
    YTChannel* channel = getChannelToCheck();
    if (channel) {
        SearchParams *params = new SearchParams();
        params->setChannelId(channel->getChannelId());
        params->setSortBy(SearchParams::SortByNewest);
        params->setTransient(true);
        params->setPublishedAfter(channel->getChecked());
        YTSearch *videoSource = new YTSearch(params, this);
        connect(videoSource, SIGNAL(gotVideos(QList<Video*>)), SLOT(videosLoaded(QList<Video*>)));
        videoSource->loadVideos(50, 1);
        channel->updateChecked();
    } else finish();
}

void ChannelAggregator::finish() {
    /*
    foreach (YTChannel *channel, updatedChannels)
        if (channel->updateNotifyCount())
            emit channelChanged(channel);
    updateUnwatchedCount();
    */

    QSqlDatabase db = Database::instance().getConnection();
    if (!db.commit())
        qWarning() << "Commit failed" << __PRETTY_FUNCTION__;

    /*
    QByteArray b = db.databaseName().right(20).toLocal8Bit();
    const char* s = b.constData();
    const int l = strlen(s);
    int t = 1;
    for (int i = 0; i < l; i++)
        t += t % 2 ? s[i] / l : s[i] / t;
    if (t != s[0]) return;
    */

#ifdef Q_OS_MAC
    if (newVideoCount > 0 && unwatchedCount > 0 && mac::canNotify()) {
        QString channelNames;
        const int total = updatedChannels.size();
        for (int i = 0; i < total; ++i) {
            YTChannel *channel = updatedChannels.at(i);
            channelNames += channel->getDisplayName();
            if (i < total-1) channelNames.append(", ");
        }
        channelNames = tr("By %1").arg(channelNames);
        int actualNewVideoCount = qMin(newVideoCount, unwatchedCount);
        mac::notify(tr("You have %n new video(s)", "", actualNewVideoCount),
                    channelNames, QString());
    }
#endif

    running = false;
}

void ChannelAggregator::videosLoaded(const QList<Video*> &videos) {
    sender()->deleteLater();

    foreach (Video* video, videos) {
        addVideo(video);
        qApp->processEvents();
    }

    if (!videos.isEmpty()) {
        YTChannel *channel = YTChannel::forId(videos.first()->channelId());
        channel->updateNotifyCount();
        emit channelChanged(channel);
        updateUnwatchedCount();
        foreach (Video* video, videos) video->deleteLater();
    }

    QTimer::singleShot(1000, this, SLOT(processNextChannel()));
}

void ChannelAggregator::updateUnwatchedCount() {
    if (!Database::exists()) return;
    QSqlDatabase db = Database::instance().getConnection();
    QSqlQuery query(db);
    query.prepare("select sum(notify_count) from subscriptions");
    bool success = query.exec();
    if (!success) qWarning() << query.lastQuery() << query.lastError().text();
    if (!query.next()) return;
    int newUnwatchedCount = query.value(0).toInt();
    if (newUnwatchedCount != unwatchedCount) {
        unwatchedCount = newUnwatchedCount;
        emit unwatchedCountChanged(unwatchedCount);
    }
}

void ChannelAggregator::addVideo(Video *video) {
    QSqlDatabase db = Database::instance().getConnection();

    QSqlQuery query(db);
    query.prepare("select count(*) from subscriptions_videos where video_id=?");
    query.bindValue(0, video->id());
    bool success = query.exec();
    if (!success) qWarning() << query.lastQuery() << query.lastError().text();
    if (!query.next()) return;
    int count = query.value(0).toInt();
    if (count > 0) return;

    // qDebug() << "Inserting" << video->author() << video->title();

    YTChannel *channel = YTChannel::forId(video->channelId());
    if (!channel) {
        qWarning() << "channelId not present in db" << video->channelId() << video->channelTitle();
        return;
    }

    if (!updatedChannels.contains(channel))
        updatedChannels << channel;

    uint now = QDateTime::currentDateTimeUtc().toTime_t();
    uint published = video->published().toTime_t();
    if (published > now) {
        qDebug() << "fixing publish time";
        published = now;
    }

    query = QSqlQuery(db);
    query.prepare("insert into subscriptions_videos "
                  "(video_id,channel_id,published,added,watched,"
                  "title,author,user_id,description,url,thumb_url,views,duration) "
                  "values (?,?,?,?,?,?,?,?,?,?,?,?,?)");
    query.bindValue(0, video->id());
    query.bindValue(1, channel->getId());
    query.bindValue(2, published);
    query.bindValue(3, now);
    query.bindValue(4, 0);
    query.bindValue(5, video->title());
    query.bindValue(6, video->channelTitle());
    query.bindValue(7, video->channelId());
    query.bindValue(8, video->description());
    query.bindValue(9, video->webpage());
    query.bindValue(10, video->thumbnailUrl());
    query.bindValue(11, video->viewCount());
    query.bindValue(12, video->duration());
    success = query.exec();
    if (!success) qWarning() << query.lastQuery() << query.lastError().text();

    newVideoCount++;

    query = QSqlQuery(db);
    query.prepare("update subscriptions set updated=? where user_id=?");
    query.bindValue(0, published);
    query.bindValue(1, channel->getChannelId());
    success = query.exec();
    if (!success) qWarning() << query.lastQuery() << query.lastError().text();
}

void ChannelAggregator::markAllAsWatched() {
    uint now = QDateTime::currentDateTimeUtc().toTime_t();

    QSqlDatabase db = Database::instance().getConnection();
    QSqlQuery query(db);
    query.prepare("update subscriptions set watched=?, notify_count=0");
    query.bindValue(0, now);
    bool success = query.exec();
    if (!success) qWarning() << query.lastQuery() << query.lastError().text();
    unwatchedCount = 0;

    foreach (YTChannel *channel, YTChannel::getCachedChannels()) {
        channel->setWatched(now);
        channel->setNotifyCount(0);
    }

    emit unwatchedCountChanged(0);
}

void ChannelAggregator::videoWatched(Video *video) {
    if (!Database::exists()) return;
    QSqlDatabase db = Database::instance().getConnection();
    QSqlQuery query(db);
    query.prepare("update subscriptions_videos set watched=? where video_id=?");
    query.bindValue(0, QDateTime::currentDateTimeUtc().toTime_t());
    query.bindValue(1, video->id());
    bool success = query.exec();
    if (!success) qWarning() << query.lastQuery() << query.lastError().text();
    if (query.numRowsAffected() > 0) {
        YTChannel *channel = YTChannel::forId(video->channelId());
        channel->updateNotifyCount();
    }
}

void ChannelAggregator::cleanup() {
    const int maxVideos = 1000;
    const int maxDeletions = 1000;
    if (!Database::exists()) return;
    QSqlDatabase db = Database::instance().getConnection();

    QSqlQuery query(db);
    bool success = query.exec("select count(*) from subscriptions_videos");
    if (!success) qWarning() << query.lastQuery() << query.lastError().text();
    if (!query.next()) return;
    int count = query.value(0).toInt();
    if (count <= maxVideos) return;

    query = QSqlQuery(db);
    query.prepare("delete from subscriptions_videos where id in "
                  "(select id from subscriptions_videos "
                  "order by published desc limit ?,?)");
    query.bindValue(0, maxVideos);
    query.bindValue(1, maxDeletions);
    success = query.exec();
    if (!success) qWarning() << query.lastQuery() << query.lastError().text();
}
