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

#ifndef YTUSER_H
#define YTUSER_H

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif
#include <QtNetwork>

class YTUser : public QObject {

    Q_OBJECT

public:
    static YTUser* forId(QString userId);
    static void subscribe(QString userId);
    static void unsubscribe(QString userId);
    static bool isSubscribed(QString userId);

    int getId() { return id; }
    void setId(int id) { this->id = id; }

    uint getChecked() { return checked; }
    void updateChecked();

    uint getWatched() const { return watched; }
    void setWatched(uint watched) { this->watched = watched; }

    int getNotifyCount() const { return notifyCount; }
    void setNotifyCount(int count) { notifyCount = count; }
    void storeNotifyCount(int count);
    bool updateNotifyCount();

    QString getUserId() const { return userId; }
    QString getUserName() const { return userName; }
    QString getDisplayName() const { return displayName; }
    QString getDescription() const { return description; }
    QString getCountryCode() const { return countryCode; }

    void loadThumbnail();
    const QString & getThumbnailDir();
    QString getThumbnailLocation();
    const QPixmap & getThumbnail() { return thumbnail; }

    static QList<YTUser*> getCachedUsers() { return cache.values(); }

public slots:
    void updateWatched();
    void unsubscribe();

signals:
    void infoLoaded();
    void thumbnailLoaded();
    void error(QString message);
    void notifyCountChanged();

private slots:
    void parseResponse(QByteArray bytes);
    void requestError(QNetworkReply *reply);
    void storeThumbnail(QByteArray bytes);

private:
    YTUser(QString userId, QObject *parent = 0);
    void maybeLoadfromAPI();
    void storeInfo();

    static QHash<QString, YTUser*> cache;

    int id;
    QString userId;
    QString userName;
    QString displayName;
    QString description;
    QString countryCode;

    QString thumbnailUrl;
    QPixmap thumbnail;
    bool loadingThumbnail;

    int notifyCount;
    uint checked;
    uint watched;
    uint loaded;
    bool loading;
};

// This is required in order to use QPointer<YTUser> as a QVariant
typedef QPointer<YTUser> YTUserPointer;
Q_DECLARE_METATYPE(YTUserPointer)

#endif // YTUSER_H
