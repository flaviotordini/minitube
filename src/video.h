#ifndef VIDEO_H
#define VIDEO_H

#include <QtGui>
#include <QtNetwork>

class Video : public QObject {

    Q_OBJECT

public:
    Video();
    Video* clone();

    const QString title() const { return m_title; }
    void setTitle( QString title ) { m_title = title; }

    const QString description() const { return m_description; }
    void setDescription( QString description ) { m_description = description; }

    const QString author() const { return m_author; }
    void setAuthor( QString author ) { m_author = author; }

    const QString authorUri() const { return m_authorUri; }
    void setAuthorUri( QString authorUri ) { m_authorUri = authorUri; }

    const QUrl webpage() const { return m_webpage; }
    void setWebpage(QUrl webpage);

    void loadThumbnail();
    const QPixmap & thumbnail() const { return m_thumbnail; }

    QString thumbnailUrl() { return m_thumbnailUrl; }
    void setThumbnailUrl(QString url) { m_thumbnailUrl = url; }

    void loadMediumThumbnail();
    QString mediumThumbnailUrl() { return m_mediumThumbnailUrl; }
    void setMediumThumbnailUrl(QString url) { m_mediumThumbnailUrl = url; }

    int duration() const { return m_duration; }
    void setDuration( int duration ) { m_duration = duration; }
    QString formattedDuration() const;

    int viewCount() const { return m_viewCount; }
    void setViewCount( int viewCount ) { m_viewCount = viewCount; }

    const QDateTime published() const { return m_published; }
    void setPublished( QDateTime published ) { m_published = published; }

    int getDefinitionCode() const { return definitionCode; }

    void loadStreamUrl();
    QUrl getStreamUrl() { return m_streamUrl; }

    QString id() const { return videoId; }

    bool operator==(const Video &other) const {
        return videoId == other.id();
    }

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
    void gotHeadHeaders(QNetworkReply*);

private:
    void getVideoInfo();
    void findVideoUrl(int definitionCode);
    void foundVideoUrl(QString videoToken, int definitionCode);

    QString m_title;
    QString m_description;
    QString m_author;
    QString m_authorUri;
    QUrl m_webpage;
    QUrl m_streamUrl;
    QPixmap m_thumbnail;
    QString m_thumbnailUrl;
    QString m_mediumThumbnailUrl;
    int m_duration;
    QDateTime m_published;
    int m_viewCount;

    QString videoId;
    QString videoToken;
    int definitionCode;

    // current index for the elTypes list
    // needed to iterate on elTypes
    int elIndex;
    
    bool loadingStreamUrl;
};

// This is required in order to use QPointer<Video> as a QVariant
// as used by the Model/View playlist
typedef QPointer<Video> VideoPointer;
Q_DECLARE_METATYPE(VideoPointer)

#endif // VIDEO_H
