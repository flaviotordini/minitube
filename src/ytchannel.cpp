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

#include "ytchannel.h"
#include "http.h"
#include "httputils.h"
#include "database.h"
#include <QtSql>

#include "yt3.h"

#include "iconutils.h"

YTChannel::YTChannel(const QString &channelId, QObject *parent) : QObject(parent),
    id(0),
    channelId(channelId),
    loadingThumbnail(false),
    notifyCount(0),
    checked(0),
    watched(0),
    loaded(0),
    loading(false) { }

QHash<QString, YTChannel*> YTChannel::cache;

YTChannel* YTChannel::forId(const QString &channelId) {
    if (channelId.isEmpty()) return 0;

    auto i = cache.constFind(channelId);
    if (i != cache.constEnd()) return i.value();

    QSqlDatabase db = Database::instance().getConnection();
    QSqlQuery query(db);
    query.prepare("select id,name,description,thumb_url,notify_count,watched,checked,loaded "
                  "from subscriptions where user_id=?");
    query.bindValue(0, channelId);
    bool success = query.exec();
    if (!success) qWarning() << query.lastQuery() << query.lastError().text();

    YTChannel* channel = 0;
    if (query.next()) {
        // Change userId to ChannelId

        channel = new YTChannel(channelId);
        channel->id = query.value(0).toInt();
        channel->displayName = query.value(1).toString();
        channel->description = query.value(2).toString();
        channel->thumbnailUrl = query.value(3).toString();
        channel->notifyCount = query.value(4).toInt();
        channel->watched = query.value(5).toUInt();
        channel->checked = query.value(6).toUInt();
        channel->loaded = query.value(7).toUInt();
        channel->thumbnail = QPixmap(channel->getThumbnailLocation());
        channel->thumbnail.setDevicePixelRatio(IconUtils::maxSupportedPixelRatio());
        channel->maybeLoadfromAPI();
        cache.insert(channelId, channel);
    }

    return channel;
}

void YTChannel::maybeLoadfromAPI() {
    if (loading) return;
    if (channelId.isEmpty()) return;

    uint now = QDateTime::currentDateTime().toTime_t();
    static const int refreshInterval = 60 * 60 * 24 * 10;
    if (loaded > now - refreshInterval) return;

    loading = true;

    QUrl url = YT3::instance().method("channels");
    QUrlQuery q(url);
    q.addQueryItem("id", channelId);
    q.addQueryItem("part", "snippet");
    url.setQuery(q);

    QObject *reply = HttpUtils::yt().get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(parseResponse(QByteArray)));
    connect(reply, SIGNAL(error(QString)), SLOT(requestError(QString)));
}

void YTChannel::parseResponse(const QByteArray &bytes) {
    QJsonDocument doc = QJsonDocument::fromJson(bytes);
    QJsonObject obj = doc.object();
    QJsonArray items = obj["items"].toArray();
    for (const QJsonValue &v : items) {
        QJsonObject item = v.toObject();
        QJsonObject snippet = item["snippet"].toObject();
        displayName = snippet["title"].toString();
        description = snippet["description"].toString();
        QJsonObject thumbnails = snippet["thumbnails"].toObject();
        thumbnailUrl = thumbnails["medium"].toObject()["url"].toString();
        qDebug() << displayName << description << thumbnailUrl;
    }

    emit infoLoaded();
    storeInfo();
    loading = false;
}

void YTChannel::loadThumbnail() {
    if (loadingThumbnail) return;
    if (thumbnailUrl.isEmpty()) return;
    loadingThumbnail = true;

    QUrl url(thumbnailUrl);
    QObject *reply = HttpUtils::yt().get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(storeThumbnail(QByteArray)));
    connect(reply, SIGNAL(error(QString)), SLOT(requestError(QString)));
}

const QString & YTChannel::getThumbnailDir() {
    static const QString thumbDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/channels/";
    return thumbDir;
}

QString YTChannel::getThumbnailLocation() {
    return getThumbnailDir() + channelId;
}

QString YTChannel::latestVideoId() {
    QSqlDatabase db = Database::instance().getConnection();
    QSqlQuery query(db);
    query.prepare("select video_id from subscriptions_videos where user_id=? order by published desc limit 1");
    query.bindValue(0, channelId);
    bool success = query.exec();
    if (!success) qWarning() << query.lastQuery() << query.lastError().text();
    if (!query.next()) return QString();
    return query.value(0).toString();
}

void YTChannel::unsubscribe() {
    YTChannel::unsubscribe(channelId);
}

void YTChannel::storeThumbnail(const QByteArray &bytes) {
    thumbnail.loadFromData(bytes);
    qreal maxRatio = IconUtils::maxSupportedPixelRatio();
    thumbnail.setDevicePixelRatio(maxRatio);
    const int maxWidth = 88 * maxRatio;

    QDir dir;
    dir.mkpath(getThumbnailDir());

    if (thumbnail.width() > maxWidth) {
        thumbnail = thumbnail.scaledToWidth(maxWidth, Qt::SmoothTransformation);
        thumbnail.setDevicePixelRatio(maxRatio);
        thumbnail.save(getThumbnailLocation(), "JPG");
    } else {
        QFile file(getThumbnailLocation());
        if (!file.open(QIODevice::WriteOnly))
            qWarning() << "Error opening file for writing" << file.fileName();
        QDataStream stream(&file);
        stream.writeRawData(bytes.constData(), bytes.size());
    }

    emit thumbnailLoaded();
    loadingThumbnail = false;
}

void YTChannel::requestError(const QString &message) {
    emit error(message);
    qWarning() << message;
    loading = false;
    loadingThumbnail = false;
}

void YTChannel::storeInfo() {
    if (channelId.isEmpty()) return;
    QSqlDatabase db = Database::instance().getConnection();
    QSqlQuery query(db);
    query.prepare("update subscriptions set "
                  "user_name=?, name=?, description=?, thumb_url=?, loaded=? "
                  "where user_id=?");
    qDebug() << userName;
    query.bindValue(0, userName);
    query.bindValue(1, displayName);
    query.bindValue(2, description);
    query.bindValue(3, thumbnailUrl);
    query.bindValue(4, QDateTime::currentDateTime().toTime_t());
    query.bindValue(5, channelId);
    bool success = query.exec();
    if (!success) qWarning() << query.lastQuery() << query.lastError().text();

    loadThumbnail();
}

void YTChannel::subscribe(const QString &channelId) {
    if (channelId.isEmpty()) return;

    uint now = QDateTime::currentDateTime().toTime_t();

    QSqlDatabase db = Database::instance().getConnection();
    QSqlQuery query(db);
    query.prepare("insert into subscriptions "
                  "(user_id,added,watched,checked,views,notify_count)"
                  " values (?,?,?,0,0,0)");
    query.bindValue(0, channelId);
    query.bindValue(1, now);
    query.bindValue(2, now);
    bool success = query.exec();
    if (!success) qWarning() << query.lastQuery() << query.lastError().text();

    // This will call maybeLoadFromApi
    YTChannel::forId(channelId);
}

void YTChannel::unsubscribe(const QString &channelId) {
    if (channelId.isEmpty()) return;
    QSqlDatabase db = Database::instance().getConnection();
    QSqlQuery query(db);
    query.prepare("delete from subscriptions where user_id=?");
    query.bindValue(0, channelId);
    bool success = query.exec();
    if (!success) qWarning() << query.lastQuery() << query.lastError().text();

    query = QSqlQuery(db);
    query.prepare("delete from subscriptions_videos where user_id=?");
    query.bindValue(0, channelId);
    success = query.exec();
    if (!success) qWarning() << query.lastQuery() << query.lastError().text();

    YTChannel *user = cache.take(channelId);
    if (user) user->deleteLater();
}

bool YTChannel::isSubscribed(const QString &channelId) {
    if (!Database::exists()) return false;
    if (channelId.isEmpty()) return false;
    QSqlDatabase db = Database::instance().getConnection();
    QSqlQuery query(db);
    query.prepare("select count(*) from subscriptions where user_id=?");
    query.bindValue(0, channelId);
    bool success = query.exec();
    if (!success) qWarning() << query.lastQuery() << query.lastError().text();
    if (query.next())
        return query.value(0).toInt() > 0;
    return false;
}

void YTChannel::updateChecked() {
    if (channelId.isEmpty()) return;

    uint now = QDateTime::currentDateTime().toTime_t();
    checked = now;

    QSqlDatabase db = Database::instance().getConnection();
    QSqlQuery query(db);
    query.prepare("update subscriptions set checked=? where user_id=?");
    query.bindValue(0, now);
    query.bindValue(1, channelId);
    bool success = query.exec();
    if (!success) qWarning() << query.lastQuery() << query.lastError().text();
}

void YTChannel::updateWatched() {
    if (channelId.isEmpty()) return;

    uint now = QDateTime::currentDateTime().toTime_t();
    watched = now;
    notifyCount = 0;
    emit notifyCountChanged();

    QSqlDatabase db = Database::instance().getConnection();
    QSqlQuery query(db);
    query.prepare("update subscriptions set watched=?, notify_count=0, views=views+1 where user_id=?");
    query.bindValue(0, now);
    query.bindValue(1, channelId);
    bool success = query.exec();
    if (!success) qWarning() << query.lastQuery() << query.lastError().text();
}

void YTChannel::storeNotifyCount(int count) {
    if (notifyCount != count)
        emit notifyCountChanged();
    notifyCount = count;

    QSqlDatabase db = Database::instance().getConnection();
    QSqlQuery query(db);
    query.prepare("update subscriptions set notify_count=? where user_id=?");
    query.bindValue(0, count);
    query.bindValue(1, channelId);
    bool success = query.exec();
    if (!success) qWarning() << query.lastQuery() << query.lastError().text();
}

bool YTChannel::updateNotifyCount() {
    QSqlDatabase db = Database::instance().getConnection();
    QSqlQuery query(db);
    query.prepare("select count(*) from subscriptions_videos "
                  "where channel_id=? and added>? and published>? and watched=0");
    query.bindValue(0, id);
    query.bindValue(1, watched);
    query.bindValue(2, watched);
    bool success = query.exec();
    if (!success) qWarning() << query.lastQuery() << query.lastError().text();
    if (!query.next()) {
        qWarning() << __PRETTY_FUNCTION__ << "Count failed";
        return false;
    }
    int count = query.value(0).toInt();
    storeNotifyCount(count);
    return count != notifyCount;
}
