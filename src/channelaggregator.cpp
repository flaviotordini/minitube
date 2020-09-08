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
#include "database.h"
#include "searchparams.h"
#include "video.h"
#include "ytchannel.h"
#include "ytsearch.h"
#ifdef APP_MAC
#include "macutils.h"
#endif
#include "http.h"
#include "httputils.h"

#include "ivchannelsource.h"
#include "videoapi.h"
#include "ytjschannelsource.h"

ChannelAggregator::ChannelAggregator(QObject *parent)
    : QObject(parent), unwatchedCount(-1), running(false), stopped(false), currentChannel(0) {
    checkInterval = 3600;

    timer = new QTimer(this);
    timer->setInterval(60000 * 5);
    connect(timer, SIGNAL(timeout()), SLOT(run()));
}

ChannelAggregator *ChannelAggregator::instance() {
    static ChannelAggregator *i = new ChannelAggregator();
    return i;
}

void ChannelAggregator::start() {
    stopped = false;
    updateUnwatchedCount();
    QTimer::singleShot(10000, this, SLOT(run()));
    if (!timer->isActive()) timer->start();
}

void ChannelAggregator::stop() {
    timer->stop();
    stopped = true;
}

YTChannel *ChannelAggregator::getChannelToCheck() {
    if (stopped) return 0;
    QSqlDatabase db = Database::instance().getConnection();
    QSqlQuery query(db);
    query.prepare("select user_id from subscriptions where checked<? "
                  "order by checked limit 1");
    query.bindValue(0, QDateTime::currentDateTimeUtc().toTime_t() - checkInterval);
    bool success = query.exec();
    if (!success) qWarning() << query.lastQuery() << query.lastError().text();
    if (query.next()) return YTChannel::forId(query.value(0).toString());
    return 0;
}

void ChannelAggregator::run() {
    if (running) return;
    if (!Database::exists()) return;
    running = true;
    newVideoCount = 0;
    updatedChannels.clear();
    updatedChannels.squeeze();

    if (!Database::instance().getConnection().transaction())
        qWarning() << "Transaction failed" << __PRETTY_FUNCTION__;

    processNextChannel();
}

void ChannelAggregator::processNextChannel() {
    if (stopped) {
        running = false;
        return;
    }
    YTChannel *channel = getChannelToCheck();
    if (channel) {
        checkWebPage(channel);
    } else
        finish();
}

void ChannelAggregator::checkWebPage(YTChannel *channel) {
    currentChannel = channel;

    QString channelId = channel->getChannelId();
    QString url;
    if (channelId.startsWith("UC") && !channelId.contains(' ')) {
        url = "https://www.youtube.com/channel/" + channelId + "/videos";
    } else {
        url = "https://www.youtube.com/user/" + channelId + "/videos";
    }

    QObject *reply = HttpUtils::yt().get(url);

    connect(reply, SIGNAL(data(QByteArray)), SLOT(parseWebPage(QByteArray)));
    connect(reply, SIGNAL(error(QString)), SLOT(errorWebPage(QString)));
}

void ChannelAggregator::parseWebPage(const QByteArray &bytes) {
    bool hasNewVideos = true;
    QRegExp re = QRegExp("[\\?&]v=([0-9A-Za-z_-]+)");
    if (re.indexIn(bytes) != -1) {
        QString videoId = re.cap(1);
        QString latestVideoId = currentChannel->latestVideoId();
        qDebug() << "Comparing" << videoId << latestVideoId;
        hasNewVideos = videoId != latestVideoId;
    } else {
        qDebug() << "Cannot capture latest video id";
    }
    if (hasNewVideos) {
        if (currentChannel) {
            reallyProcessChannel(currentChannel);
            currentChannel = 0;
        }
    } else {
        currentChannel->updateChecked();
        currentChannel = 0;
        processNextChannel();
    }
}

void ChannelAggregator::errorWebPage(const QString &message) {
    Q_UNUSED(message);
    reallyProcessChannel(currentChannel);
    currentChannel = 0;
}

void ChannelAggregator::reallyProcessChannel(YTChannel *channel) {
    SearchParams *params = new SearchParams();
    params->setChannelId(channel->getChannelId());
    params->setSortBy(SearchParams::SortByNewest);
    params->setTransient(true);
    params->setPublishedAfter(channel->getChecked());

    if (VideoAPI::impl() == VideoAPI::YT3) {
        YTSearch *videoSource = new YTSearch(params);
        connect(videoSource, SIGNAL(gotVideos(QVector<Video *>)),
                SLOT(videosLoaded(QVector<Video *>)));
        videoSource->loadVideos(50, 1);
    } else if (VideoAPI::impl() == VideoAPI::IV) {
        auto *videoSource = new IVChannelSource(params);
        connect(videoSource, SIGNAL(gotVideos(QVector<Video *>)),
                SLOT(videosLoaded(QVector<Video *>)));
        videoSource->loadVideos(50, 1);
    } else if (VideoAPI::impl() == VideoAPI::JS) {
        auto *videoSource = new YTJSChannelSource(params);
        connect(videoSource, SIGNAL(gotVideos(QVector<Video *>)),
                SLOT(videosLoaded(QVector<Video *>)));
        videoSource->loadVideos(50, 1);
    }

    channel->updateChecked();
}

void ChannelAggregator::finish() {
    currentChannel = 0;

    QSqlDatabase db = Database::instance().getConnection();
    if (!db.commit()) qWarning() << "Commit failed" << __PRETTY_FUNCTION__;

#ifdef Q_OS_MAC
    if (newVideoCount > 0 && unwatchedCount > 0 && mac::canNotify()) {
        QString channelNames;
        const int total = updatedChannels.size();
        for (int i = 0; i < total; ++i) {
            YTChannel *channel = updatedChannels.at(i);
            channelNames += channel->getDisplayName();
            if (i < total - 1) channelNames.append(", ");
        }
        channelNames = tr("By %1").arg(channelNames);
        int actualNewVideoCount = qMin(newVideoCount, unwatchedCount);
        mac::notify(tr("You have %n new video(s)", "", actualNewVideoCount), channelNames,
                    QString());
    }
#endif

    running = false;
}

void ChannelAggregator::videosLoaded(const QVector<Video *> &videos) {
    sender()->deleteLater();

    for (Video *video : videos) {
        addVideo(video);
        qApp->processEvents();
    }

    if (!videos.isEmpty()) {
        YTChannel *channel = YTChannel::forId(videos.at(0)->getChannelId());
        channel->updateNotifyCount();
        emit channelChanged(channel);
        updateUnwatchedCount();
        for (Video *video : videos)
            video->deleteLater();
    }

    QTimer::singleShot(0, this, SLOT(processNextChannel()));
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
    query.bindValue(0, video->getId());
    bool success = query.exec();
    if (!success) qWarning() << query.lastQuery() << query.lastError().text();
    if (!query.next()) return;
    int count = query.value(0).toInt();
    if (count > 0) return;

    // qDebug() << "Inserting" << video->author() << video->title();

    YTChannel *channel = YTChannel::forId(video->getChannelId());
    if (!channel) {
        qWarning() << "channelId not present in db" << video->getChannelId()
                   << video->getChannelTitle();
        return;
    }

    if (!updatedChannels.contains(channel)) updatedChannels << channel;

    uint now = QDateTime::currentDateTimeUtc().toTime_t();
    uint published = video->getPublished().toTime_t();
    if (published > now) {
        qDebug() << "fixing publish time";
        published = now;
    }

    query = QSqlQuery(db);
    query.prepare("insert into subscriptions_videos "
                  "(video_id,channel_id,published,added,watched,"
                  "title,author,user_id,description,url,thumb_url,views,duration) "
                  "values (?,?,?,?,?,?,?,?,?,?,?,?,?)");
    query.bindValue(0, video->getId());
    query.bindValue(1, channel->getId());
    query.bindValue(2, published);
    query.bindValue(3, now);
    query.bindValue(4, 0);
    query.bindValue(5, video->getTitle());
    query.bindValue(6, video->getChannelTitle());
    query.bindValue(7, video->getChannelId());
    query.bindValue(8, video->getDescription());
    query.bindValue(9, video->getWebpage());
    query.bindValue(10, video->getThumbnailUrl());
    query.bindValue(11, video->getViewCount());
    query.bindValue(12, video->getDuration());
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

    const auto &channels = YTChannel::getCachedChannels();
    for (YTChannel *channel : channels) {
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
    query.bindValue(1, video->getId());
    bool success = query.exec();
    if (!success) qWarning() << query.lastQuery() << query.lastError().text();
    if (query.numRowsAffected() > 0) {
        YTChannel *channel = YTChannel::forId(video->getChannelId());
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
