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

#ifndef VIDEO_H
#define VIDEO_H

#include <QtCore>
#include <QtGui>

class YTVideo;
class YTJSVideo;

class Video : public QObject {
    Q_OBJECT

public:
    Video();
    ~Video();
    Video *clone();

    enum License { LicenseYouTube = 1, LicenseCC };
    Q_ENUM(License)

    const QString &getTitle() const { return title; }
    void setTitle(const QString &value) { title = value; }

    const QString &getDescription() const { return description; }
    void setDescription(const QString &value) { description = value; }

    const QString &getChannelTitle() const { return channelTitle; }
    void setChannelTitle(const QString &value) { channelTitle = value; }

    const QString &getChannelId() const { return channelId; }
    void setChannelId(const QString &value) { channelId = value; }

    const QString &getWebpage();
    void setWebpage(const QString &value);

    void loadThumbnail();
    const QPixmap &getThumbnail() const { return thumbnail; }

    const QString &getThumbnailUrl() const { return thumbnailUrl; }
    void setThumbnailUrl(const QString &value) { thumbnailUrl = value; }

    const QString &getMediumThumbnailUrl() const { return mediumThumbnailUrl; }
    void setMediumThumbnailUrl(const QString &value) { mediumThumbnailUrl = value; }

    const QString &getLargeThumbnailUrl() const { return largeThumbnailUrl; }
    void setLargeThumbnailUrl(const QString &value) { largeThumbnailUrl = value; }

    int getDuration() const { return duration; }
    void setDuration(int value);
    const QString &getFormattedDuration() const { return formattedDuration; }

    int getViewCount() const { return viewCount; }
    void setViewCount(int value);
    const QString &getFormattedViewCount() const { return formattedViewCount; }

    const QDateTime &getPublished() const { return published; }
    void setPublished(const QDateTime &value);
    const QString &getFormattedPublished() const { return formattedPublished; }

    int getDefinitionCode() const { return definitionCode; }

    void loadStreamUrl();
    const QString &getStreamUrl() { return streamUrl; }
    bool isLoadingStreamUrl() const { return ytVideo != nullptr; }
    void abortLoadStreamUrl();

    const QString &getId() const { return id; }
    void setId(const QString &value) { id = value; }

    License getLicense() const { return license; }
    void setLicense(License value) { license = value; }

signals:
    void gotThumbnail();
    void gotMediumThumbnail(const QByteArray &bytes);
    void gotLargeThumbnail(const QByteArray &bytes);
    void gotStreamUrl(const QString &videoUrl, const QString &audioUrl);
    void errorStreamUrl(const QString &message);

private slots:
    void setThumbnail(const QByteArray &bytes);
    void streamUrlLoaded(const QString &streamUrl, const QString &audioUrl);

private:
    void loadStreamUrlJS();

    QString title;
    QString description;
    QString channelTitle;
    QString channelId;
    QString webpage;
    QString streamUrl;
    QPixmap thumbnail;
    QString thumbnailUrl;
    QString mediumThumbnailUrl;
    QString largeThumbnailUrl;
    int duration;
    QString formattedDuration;

    QDateTime published;
    QString formattedPublished;
    int viewCount;
    QString formattedViewCount;
    License license;
    QString id;
    int definitionCode;

    bool loadingThumbnail;

    YTVideo *ytVideo;
    YTJSVideo *ytjsVideo;
};

// This is required in order to use QPointer<Video> as a QVariant
// as used by the Model/View playlist
typedef QPointer<Video> VideoPointer;
Q_DECLARE_METATYPE(VideoPointer)

#endif // VIDEO_H
