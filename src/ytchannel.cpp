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
#include "networkaccess.h"
#include "database.h"
#include <QtSql>

#ifdef APP_YT3
#include "yt3.h"
#include <QtScript>
#endif

namespace The {
NetworkAccess* http();
}

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

    if (cache.contains(channelId))
        return cache.value(channelId);

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

#ifdef APP_YT3

    QUrl url = YT3::instance().method("channels");
#if QT_VERSION >= 0x050000
    {
        QUrl &u = url;
        QUrlQuery url;
#endif
        url.addQueryItem("id", channelId);
        url.addQueryItem("part", "snippet");
#if QT_VERSION >= 0x050000
        u.setQuery(url);
    }
#endif

#else

    QUrl url("http://gdata.youtube.com/feeds/api/users/" + channelId);
#if QT_VERSION >= 0x050000
    {
        QUrl &u = url;
        QUrlQuery url;
#endif
        url.addQueryItem("v", "2");
#if QT_VERSION >= 0x050000
        u.setQuery(url);
    }
#endif

#endif

    QObject *reply = The::http()->get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(parseResponse(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), SLOT(requestError(QNetworkReply*)));
}

#ifdef APP_YT3

void YTChannel::parseResponse(const QByteArray &bytes) {
    QScriptEngine engine;
    QScriptValue json = engine.evaluate("(" + QString::fromUtf8(bytes) + ")");
    QScriptValue items = json.property("items");
    if (items.isArray()) {
        QScriptValueIterator it(items);
        while (it.hasNext()) {
            it.next();
            QScriptValue item = it.value();
            // For some reason the array has an additional element containing its size.
            if (item.isObject()) {
                QScriptValue snippet = item.property("snippet");
                displayName = snippet.property("title").toString();
                description = snippet.property("description").toString();
                QScriptValue thumbnails = snippet.property("thumbnails");
                thumbnailUrl = thumbnails.property("default").property("url").toString();
                qDebug() << displayName << description << thumbnailUrl;
            }
        }
    }

    emit infoLoaded();
    storeInfo();
    loading = false;
}

#else

void YTChannel::parseResponse(const QByteArray &bytes) {
    QXmlStreamReader xml(bytes);
    xml.readNextStartElement();
    if (xml.name() == QLatin1String("entry"))
        while(xml.readNextStartElement()) {
            const QStringRef n = xml.name();
            if (n == QLatin1String("summary"))
                description = xml.readElementText().simplified();
            else if (n == QLatin1String("title"))
                displayName = xml.readElementText();
            else if (n == QLatin1String("thumbnail")) {
                thumbnailUrl = xml.attributes().value("url").toString();
                xml.skipCurrentElement();
            } else if (n == QLatin1String("username"))
                userName = xml.readElementText();
            else xml.skipCurrentElement();
        }

    if (xml.hasError()) {
        emit error(xml.errorString());
        qWarning() << xml.errorString();
    }

    emit infoLoaded();
    storeInfo();
    loading = false;
}

#endif

void YTChannel::loadThumbnail() {
    if (loadingThumbnail) return;
    if (thumbnailUrl.isEmpty()) return;
    loadingThumbnail = true;

#ifdef Q_OS_WIN
    thumbnailUrl.replace(QLatin1String("https://"), QLatin1String("http://"));
#endif

    QUrl url(thumbnailUrl);
    QObject *reply = The::http()->get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(storeThumbnail(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), SLOT(requestError(QNetworkReply*)));
}

const QString & YTChannel::getThumbnailDir() {
    static const QString thumbDir =
        #if QT_VERSION >= 0x050000
            QStandardPaths::writableLocation(QStandardPaths::DataLocation)
        #else
            QDesktopServices::storageLocation(QDesktopServices::DataLocation)
        #endif
            + "/channels/";
    return thumbDir;
}

QString YTChannel::getThumbnailLocation() {
    return getThumbnailDir() + channelId;
}

void YTChannel::unsubscribe() {
    YTChannel::unsubscribe(channelId);
}

void YTChannel::storeThumbnail(const QByteArray &bytes) {
    thumbnail.loadFromData(bytes);
    static const int maxWidth = 88;

    QDir dir;
    dir.mkpath(getThumbnailDir());

    if (thumbnail.width() > maxWidth) {
        thumbnail = thumbnail.scaledToWidth(maxWidth, Qt::SmoothTransformation);
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

void YTChannel::requestError(QNetworkReply *reply) {
    emit error(reply->errorString());
    qWarning() << reply->errorString();
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
