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

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif
#include <QtNetwork>

class Video : public QObject {

    Q_OBJECT

public:
    Video();
    Video* clone();

    enum License {
        LicenseYouTube = 1,
        LicenseCC
    };

    const QString & title() const { return m_title; }
    void setTitle( QString title ) { m_title = title; }

    const QString & description() const { return m_description; }
    void setDescription( QString description ) { m_description = description; }

    const QString & author() const { return m_author; }
    void setAuthor( QString author ) { m_author = author; }

    const QString & userId() const { return m_userId; }
    void setUserId( QString userId ) { m_userId = userId; }

    const QUrl & webpage() const { return m_webpage; }
    void setWebpage(QUrl webpage);

    void loadThumbnail();
    const QPixmap & thumbnail() const { return m_thumbnail; }

    const QString & thumbnailUrl() { return m_thumbnailUrl; }
    void setThumbnailUrl(QString url) { m_thumbnailUrl = url; }

    void loadMediumThumbnail();
    const QString & mediumThumbnailUrl() { return m_mediumThumbnailUrl; }
    void setMediumThumbnailUrl(QString url) { m_mediumThumbnailUrl = url; }

    int duration() const { return m_duration; }
    void setDuration( int duration ) { m_duration = duration; }
    QString formattedDuration() const;

    int viewCount() const { return m_viewCount; }
    void setViewCount( int viewCount ) { m_viewCount = viewCount; }

    const QDateTime & published() const { return m_published; }
    void setPublished( QDateTime published ) { m_published = published; }

    int getDefinitionCode() const { return definitionCode; }

    void loadStreamUrl();
    const QUrl & getStreamUrl() { return m_streamUrl; }

    void setId(QString id) { videoId = id; }
    const QString & id() const { return videoId; }

    void setLicense(License license) { m_license = license; }
    License license() const { return m_license; }

signals:
    void gotThumbnail();
    void gotMediumThumbnail(QByteArray bytes);
    void gotStreamUrl(QUrl streamUrl);
    void errorStreamUrl(QString message);

private slots:
    void setThumbnail(QByteArray bytes);
    void gotVideoInfo(QByteArray);
    void errorVideoInfo(QNetworkReply*);
    void scrapeWebPage(QByteArray);
    void parseJsPlayer(QByteArray bytes);
    void parseDashManifest(QByteArray bytes);

private:
    void getVideoInfo();
    void parseFmtUrlMap(const QString &fmtUrlMap, bool fromWebPage = false);
    void captureFunction(const QString &name, const QString &js);
    void captureObject(const QString &name, const QString &js);
    QString decryptSignature(const QString &s);

    QString m_title;
    QString m_description;
    QString m_author;
    QString m_userId;
    QUrl m_webpage;
    QUrl m_streamUrl;
    QPixmap m_thumbnail;
    QString m_thumbnailUrl;
    QString m_mediumThumbnailUrl;
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
};

// This is required in order to use QPointer<Video> as a QVariant
// as used by the Model/View playlist
typedef QPointer<Video> VideoPointer;
Q_DECLARE_METATYPE(VideoPointer)

#endif // VIDEO_H
