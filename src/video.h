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

#include <QtWidgets>
#include <QtNetwork>

class VideoDefinition;

class Video : public QObject {

    Q_OBJECT

public:
    Video();
    Video* clone();

    enum License {
        LicenseYouTube = 1,
        LicenseCC
    };

    const QString &title() const { return m_title; }
    void setTitle(const QString &value) { m_title = value; }

    const QString &description() const { return m_description; }
    void setDescription(const QString &value) { m_description = value; }

    const QString &channelTitle() const { return m_channelTitle; }
    void setChannelTitle(const QString &value) { m_channelTitle = value; }

    const QString &channelId() const { return m_channelId; }
    void setChannelId(const QString &value ) { m_channelId = value; }

    const QString &webpage();
    void setWebpage(const QString &value);

    void loadThumbnail();
    const QPixmap &thumbnail() const { return m_thumbnail; }

    const QString &thumbnailUrl() { return m_thumbnailUrl; }
    void setThumbnailUrl(const QString &value) { m_thumbnailUrl = value; }

    const QString &mediumThumbnailUrl() { return m_mediumThumbnailUrl; }
    void setMediumThumbnailUrl(const QString &value) { m_mediumThumbnailUrl = value; }

    const QString &largeThumbnailUrl() { return m_largeThumbnailUrl; }
    void setLargeThumbnailUrl(const QString &value) { m_largeThumbnailUrl = value; }

    int duration() const { return m_duration; }
    void setDuration(int value) { m_duration = value; }
    QString formattedDuration() const;

    int viewCount() const { return m_viewCount; }
    void setViewCount(int viewCount) { m_viewCount = viewCount; }

    const QDateTime &published() const { return m_published; }
    void setPublished(const QDateTime &value) { m_published = value; }

    int getDefinitionCode() const { return definitionCode; }

    void loadStreamUrl();
    const QUrl &getStreamUrl() { return m_streamUrl; }

    void setId(const QString &value) { videoId = value; }
    const QString &id() const { return videoId; }

    void setLicense(License value) { m_license = value; }
    License license() const { return m_license; }

signals:
    void gotThumbnail();
    void gotMediumThumbnail(const QByteArray &bytes);
    void gotLargeThumbnail(const QByteArray &bytes);
    void gotStreamUrl(const QUrl &streamUrl);
    void errorStreamUrl(const QString &message);

private slots:
    void setThumbnail(const QByteArray &bytes);
    void gotVideoInfo(const QByteArray &bytes);
    void errorVideoInfo(const QString &message);
    void scrapeWebPage(const QByteArray &bytes);
    void parseJsPlayer(const QByteArray &bytes);
    void parseDashManifest(const QByteArray &bytes);

private:
    void getVideoInfo();
    void parseFmtUrlMap(const QString &fmtUrlMap, bool fromWebPage = false);
    void captureFunction(const QString &name, const QString &js);
    void captureObject(const QString &name, const QString &js);
    QString decryptSignature(const QString &s);
    void saveDefinitionForUrl(const QString &url, const VideoDefinition &definition);

    QString m_title;
    QString m_description;
    QString m_channelTitle;
    QString m_channelId;
    QString m_webpage;
    QUrl m_streamUrl;
    QPixmap m_thumbnail;
    QString m_thumbnailUrl;
    QString m_mediumThumbnailUrl;
    QString m_largeThumbnailUrl;
    int m_duration;
    QDateTime m_published;
    int m_viewCount;
    License m_license;
    QString videoId;
    QString videoToken;
    int definitionCode;

    // current index for the elTypes list
    // needed to iterate on elTypes
    int elIndex;
    bool ageGate;
    
    bool loadingStreamUrl;
    bool loadingThumbnail;

    QString fmtUrlMap;
    QString sigFuncName;
    QHash<QString, QString> sigFunctions;
    QHash<QString, QString> sigObjects;

    QString dashManifestUrl;
    QString jsPlayer;
};

// This is required in order to use QPointer<Video> as a QVariant
// as used by the Model/View playlist
typedef QPointer<Video> VideoPointer;
Q_DECLARE_METATYPE(VideoPointer)

#endif // VIDEO_H
