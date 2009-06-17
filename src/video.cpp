#include "video.h"
#include "networkaccess.h"
#include <QtNetwork>

namespace The {
    NetworkAccess* http();
}

Video::Video() : m_thumbnailUrls(QList<QUrl>()) {
    m_duration = 0;
    m_viewCount = -1;
}

void Video::preloadThumbnail() {
    if (m_thumbnailUrls.isEmpty()) return;
    QObject *reply = The::http()->get(m_thumbnailUrls.first());
    connect(reply, SIGNAL(data(QByteArray)), SLOT(setThumbnail(QByteArray)));
}

void Video::setThumbnail(QByteArray bytes) {
    m_thumbnail = QImage::fromData(bytes);
    emit gotThumbnail();
}

const QImage Video::thumbnail() const {
    return m_thumbnail;
}

bool Video::getVideoUrl() {

    // https://develop.participatoryculture.org/trac/democracy/browser/trunk/tv/portable/flashscraper.py

    QUrl webpage = m_webpage;\
    // if (webpage == ) return false;
    // qDebug() << webpage.toString();

    // Get Video ID
    // youtube-dl line 428
    // QRegExp re("^((?:http://)?(?:\\w+\\.)?youtube\\.com/(?:(?:v/)|(?:(?:watch(?:\\.php)?)?\\?(?:.+&)?v=)))?([0-9A-Za-z_-]+)(?(1).+)?$");
    QRegExp re("^http://www\\.youtube\\.com/watch\\?v=([0-9A-Za-z_-]+)$");
    bool match = re.exactMatch(webpage.toString());
    if (!match || re.numCaptures() < 1) return false;
    QString videoId = re.cap(1);
    // if (!videoId) return false;
    // qDebug() << videoId;

    // Get Video Token
    QUrl normalizedUrl = QUrl(QString("http://www.youtube.com/get_video_info?video_id=")
                              .append(videoId).append("&el=embedded&ps=default&eurl="));

    /*
    QObject *reply = The::http()->get(normalizedUrl);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(gotVideoInfo(QByteArray)));
    return true;
    */

    QString videoInfo = The::http()->syncGetString(normalizedUrl);
    // qDebug() << videoInfo;
    re = QRegExp("^.*&token=([^&]+).*$");
    match = re.exactMatch(videoInfo);
    if (!match || re.numCaptures() < 1) return false;
    QString videoToken = re.cap(1);
    // FIXME proper decode
    videoToken = videoToken.replace("%3D", "=");
    // qDebug() << "token" << videoToken;

    m_streamUrl = QUrl(QString("http://www.youtube.com/get_video?video_id=")
                         .append(videoId)
                         .append("&t=").append(videoToken)
                         .append("&eurl=&el=embedded&ps=default&fmt=18"));

    // qDebug() << videoUrl;

    // follow redirects
    /*
    while (true) {
        QHttpResponseHeader headers = syncHttp->head(videoUrl);
        qDebug() << headers.values();
        if (headers.hasKey("Location")) {
            videoUrl = QUrl(headers.value("Location"), QUrl::StrictMode);
            // qDebug() << videoUrl;
        } else break;
    }*/

    return true;
}


/*
void  Video::gotVideoInfo(QByteArray data) {
    QString videoInfo = QString::fromUtf8(data);
    // qDebug() << videoInfo;
    QRegExp re = QRegExp("^.*&token=([^&]+).*$");
    bool match = re.exactMatch(videoInfo);
    if (!match || re.numCaptures() < 1) return;
    QString videoToken = re.cap(1);
    // FIXME proper decode
    videoToken = videoToken.replace("%3D", "=");
    // qDebug() << "token" << videoToken;

    QUrl videoUrl = QUrl(QString("http://www.youtube.com/get_video?video_id=")
                         .append(videoId)
                         .append("&t=").append(videoToken)
                         .append("&eurl=&el=embedded&ps=default&fmt=18"));

    m_streamUrl = videoUrl;
}
*/
