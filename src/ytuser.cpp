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

#include "ytuser.h"
#include "networkaccess.h"
#include "database.h"
#include <QtSql>

namespace The {
NetworkAccess* http();
}

YTUser::YTUser(QString userId, QObject *parent) : QObject(parent),
    id(0),
    userId(userId),
    loadingThumbnail(false),
    notifyCount(0),
    checked(0),
    watched(0),
    loaded(0),
    loading(false) { }

QHash<QString, YTUser*> YTUser::cache;

YTUser* YTUser::forId(QString userId) {
    if (userId.isEmpty()) return 0;

    if (cache.contains(userId))
        return cache.value(userId);

    QSqlDatabase db = Database::instance().getConnection();
    QSqlQuery query(db);
    query.prepare("select id,name,description,thumb_url,notify_count,watched,checked,loaded "
                  "from subscriptions where user_id=?");
    query.bindValue(0, userId);
    bool success = query.exec();
    if (!success) qWarning() << query.lastQuery() << query.lastError().text();

    YTUser* user = 0;
    if (query.next()) {
        user = new YTUser(userId);
        user->id = query.value(0).toInt();
        user->displayName = query.value(1).toString();
        user->description = query.value(2).toString();
        user->thumbnailUrl = query.value(3).toString();
        user->notifyCount = query.value(4).toInt();
        user->watched = query.value(5).toUInt();
        user->checked = query.value(6).toUInt();
        user->loaded = query.value(7).toUInt();
        user->thumbnail = QPixmap(user->getThumbnailLocation());
        user->maybeLoadfromAPI();
        cache.insert(userId, user);
    }

    return user;
}

void YTUser::maybeLoadfromAPI() {
    if (loading) return;
    if (userId.isEmpty()) return;

    uint now = QDateTime::currentDateTime().toTime_t();
    static const int refreshInterval = 60 * 60 * 24 * 10;
    if (loaded > now - refreshInterval) return;

    loading = true;

    QUrl url("http://gdata.youtube.com/feeds/api/users/" + userId);
    url.addQueryItem("v", "2");
    QObject *reply = The::http()->get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(parseResponse(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), SLOT(requestError(QNetworkReply*)));
}

void YTUser::parseResponse(QByteArray bytes) {
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

void YTUser::loadThumbnail() {
    if (loadingThumbnail) return;
    if (thumbnailUrl.isEmpty()) return;
    loadingThumbnail = true;

#ifdef Q_WS_WIN
    thumbnailUrl.replace(QLatin1String("https://"), QLatin1String("http://"));
#endif

    QUrl url(thumbnailUrl);
    QObject *reply = The::http()->get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(storeThumbnail(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), SLOT(requestError(QNetworkReply*)));
}

const QString & YTUser::getThumbnailDir() {
    static const QString thumbDir = QDesktopServices::storageLocation(
                QDesktopServices::DataLocation) + "/channels/";
    return thumbDir;
}

QString YTUser::getThumbnailLocation() {
    return getThumbnailDir() + userId;
}

void YTUser::storeThumbnail(QByteArray bytes) {
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

void YTUser::requestError(QNetworkReply *reply) {
    emit error(reply->errorString());
    qWarning() << reply->errorString();
    loading = false;
    loadingThumbnail = false;
}

void YTUser::storeInfo() {
    if (userId.isEmpty()) return;
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
    query.bindValue(5, userId);
    bool success = query.exec();
    if (!success) qWarning() << query.lastQuery() << query.lastError().text();

    loadThumbnail();
}

void YTUser::subscribe(QString userId) {
    if (userId.isEmpty()) return;

    uint now = QDateTime::currentDateTime().toTime_t();

    QSqlDatabase db = Database::instance().getConnection();
    QSqlQuery query(db);
    query.prepare("insert into subscriptions "
                  "(user_id,added,watched,checked,views,notify_count)"
                  " values (?,?,?,0,0,0)");
    query.bindValue(0, userId);
    query.bindValue(1, now);
    query.bindValue(2, now);
    bool success = query.exec();
    if (!success) qWarning() << query.lastQuery() << query.lastError().text();

    // This will call maybeLoadFromApi
    YTUser::forId(userId);
}

void YTUser::unsubscribe(QString userId) {
    if (userId.isEmpty()) return;
    QSqlDatabase db = Database::instance().getConnection();
    QSqlQuery query(db);
    query.prepare("delete from subscriptions where user_id=?");
    query.bindValue(0, userId);
    bool success = query.exec();
    if (!success) qWarning() << query.lastQuery() << query.lastError().text();

    query = QSqlQuery(db);
    query.prepare("delete from subscriptions_videos where user_id=?");
    query.bindValue(0, userId);
    success = query.exec();
    if (!success) qWarning() << query.lastQuery() << query.lastError().text();

    YTUser *user = cache.take(userId);
    if (user) user->deleteLater();
}

bool YTUser::isSubscribed(QString userId) {
    if (!Database::exists()) return false;
    if (userId.isEmpty()) return false;
    QSqlDatabase db = Database::instance().getConnection();
    QSqlQuery query(db);
    query.prepare("select count(*) from subscriptions where user_id=?");
    query.bindValue(0, userId);
    bool success = query.exec();
    if (!success) qWarning() << query.lastQuery() << query.lastError().text();
    if (query.next())
        return query.value(0).toInt() > 0;
    return false;
}

void YTUser::updateChecked() {
    if (userId.isEmpty()) return;

    uint now = QDateTime::currentDateTime().toTime_t();
    checked = now;

    QSqlDatabase db = Database::instance().getConnection();
    QSqlQuery query(db);
    query.prepare("update subscriptions set checked=? where user_id=?");
    query.bindValue(0, QDateTime::currentDateTime().toTime_t());
    query.bindValue(1, userId);
    bool success = query.exec();
    if (!success) qWarning() << query.lastQuery() << query.lastError().text();
}

void YTUser::updateWatched() {
    if (userId.isEmpty()) return;

    uint now = QDateTime::currentDateTime().toTime_t();
    watched = now;
    notifyCount = 0;

    QSqlDatabase db = Database::instance().getConnection();
    QSqlQuery query(db);
    query.prepare("update subscriptions set watched=?, notify_count=0, views=views+1 where user_id=?");
    query.bindValue(0, now);
    query.bindValue(1, userId);
    bool success = query.exec();
    if (!success) qWarning() << query.lastQuery() << query.lastError().text();
}

void YTUser::storeNotifyCount(int count) {
    notifyCount = count;

    QSqlDatabase db = Database::instance().getConnection();
    QSqlQuery query(db);
    query.prepare("update subscriptions set notify_count=? where user_id=?");
    query.bindValue(0, count);
    query.bindValue(1, userId);
    bool success = query.exec();
    if (!success) qWarning() << query.lastQuery() << query.lastError().text();
}

bool YTUser::updateNotifyCount() {
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
