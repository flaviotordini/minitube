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

#ifndef YTCHANNEL_H
#define YTCHANNEL_H

#include <QtWidgets>
#include <QtNetwork>

class YTChannel : public QObject {

    Q_OBJECT

public:
    static YTChannel* forId(const QString &channelId);
    static bool subscribe(const QString &channelId);
    static void unsubscribe(const QString &channelId);
    static bool isSubscribed(const QString &channelId);

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

    QString getChannelId() const { return channelId; }
    QString getUserName() const { return userName; }
    QString getDisplayName() const { return displayName; }
    QString getDescription() const { return description; }
    QString getCountryCode() const { return countryCode; }

    void loadThumbnail();
    const QString & getThumbnailDir();
    QString getThumbnailLocation();
    const QPixmap & getThumbnail() { return thumbnail; }

    QString latestVideoId();

    static const QHash<QString, YTChannel*> &getCachedChannels() { return cache; }

public slots:
    void updateWatched();
    void unsubscribe();

signals:
    void infoLoaded();
    void thumbnailLoaded();
    void error(QString message);
    void notifyCountChanged();

private slots:
    void parseResponse(const QByteArray &bytes);
    void requestError(const QString &message);
    void storeThumbnail(const QByteArray &bytes);

private:
    YTChannel(const QString &channelId, QObject *parent = 0);
    void maybeLoadfromAPI();
    void storeInfo();

    static QHash<QString, YTChannel*> cache;

    int id;
    QString channelId;
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
typedef QPointer<YTChannel> YTChannelPointer;
Q_DECLARE_METATYPE(YTChannelPointer)

#endif // YTCHANNEL_H
