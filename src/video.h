#ifndef VIDEO_H
#define VIDEO_H

#include <QtGui>
#include <QtNetwork>

class Video : public QObject {

    Q_OBJECT

public:
    Video();

    const QString title() const { return m_title; }
    void setTitle( QString title ) { m_title = title; }

    const QString description() const { return m_description; }
    void setDescription( QString description ) { m_description = description; }

    const QString author() const { return m_author; }
    void setAuthor( QString author ) { m_author = author; }

    const QUrl webpage() const { return m_webpage; }
    void setWebpage( QUrl webpage ) { m_webpage = webpage; }

    void loadStreamUrl() {
        if (m_streamUrl.isEmpty())
            this->scrapeStreamUrl();
        else emit gotStreamUrl(m_streamUrl);
    }

    QList<QUrl> thumbnailUrls() const { return m_thumbnailUrls; }
    void addThumbnailUrl(QUrl url) {
        m_thumbnailUrls << url;
    }

    void preloadThumbnail();
    const QImage thumbnail() const;

    int duration() const { return m_duration; }
    void setDuration( int duration ) { m_duration = duration; }

    int viewCount() const { return m_viewCount; }
    void setViewCount( int viewCount ) { m_viewCount = viewCount; }

    const QDateTime published() const { return m_published; }
    void setPublished( QDateTime published ) { m_published = published; }

public slots:
    void setThumbnail(QByteArray bytes);

signals:
    void gotThumbnail();
    void gotStreamUrl(QUrl streamUrl);

private slots:
    void gotVideoInfo(QByteArray);

private:
    void scrapeStreamUrl();

    QString m_title;
    QString m_description;
    QString m_author;
    // QUrl m_authorUrl;
    QUrl m_webpage;
    QUrl m_streamUrl;
    QImage m_thumbnail;
    QList<QUrl> m_thumbnailUrls;
    // QList<QImage> m_thumbnails;
    int m_duration;
    QDateTime m_published;
    int m_viewCount;

    // The YouTube video id
    // This is needed by the gotVideoInfo callback
    QString videoId;
};

// This is required in order to use QPointer<Video> as a QVariant
// as used by the Model/View playlist
typedef QPointer<Video> VideoPointer;
Q_DECLARE_METATYPE(VideoPointer)

#endif // VIDEO_H
