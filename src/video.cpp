#include "video.h"
#include "networkaccess.h"
#include <QtNetwork>

namespace The {
    NetworkAccess* http();
}

Video::Video() : m_duration(0),
m_viewCount(-1),
m_hd(false) { }

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

void Video::loadStreamUrl() {
    // if (m_streamUrl.isEmpty())
        this->scrapeStreamUrl();
    // else emit gotStreamUrl(m_streamUrl);
}

void Video::scrapeStreamUrl() {

    // https://develop.participatoryculture.org/trac/democracy/browser/trunk/tv/portable/flashscraper.py

    QUrl webpage = m_webpage;
    // qDebug() << webpage.toString();

    // Get Video ID
    // youtube-dl line 428
    // QRegExp re("^((?:http://)?(?:\\w+\\.)?youtube\\.com/(?:(?:v/)|(?:(?:watch(?:\\.php)?)?\\?(?:.+&)?v=)))?([0-9A-Za-z_-]+)(?(1).+)?$");
    QRegExp re("^http://www\\.youtube\\.com/watch\\?v=([0-9A-Za-z_-]+).*");
    bool match = re.exactMatch(webpage.toString());
    if (!match || re.numCaptures() < 1) {
        emit errorStreamUrl(QString("Cannot get video id for %1").arg(webpage.toString()));
        return;
    }
    videoId = re.cap(1);
    // if (!videoId) return false;
    // qDebug() << videoId;

    // Get Video Token
    QUrl normalizedUrl = QUrl(QString("http://www.youtube.com/get_video_info?video_id=")
                              .append(videoId).append("&el=embedded&ps=default&eurl="));

    QObject *reply = The::http()->get(normalizedUrl);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(gotVideoInfo(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), SLOT(errorVideoInfo(QNetworkReply*)));

    // see you in gotVideoInfo...
}

void  Video::gotVideoInfo(QByteArray data) {
    QString videoInfo = QString::fromUtf8(data);

    QRegExp re = QRegExp("^.*&token=([^&]+).*$");
    bool match = re.exactMatch(videoInfo);

    // on regexp failure, stop and report error
    if (!match || re.numCaptures() < 1) {

        // Don't panic! We have a plan B.

        // get the youtube video webpage
        QObject *reply = The::http()->get(webpage().toString());
        connect(reply, SIGNAL(data(QByteArray)), SLOT(scrapWebPage(QByteArray)));
        connect(reply, SIGNAL(error(QNetworkReply*)), SLOT(errorVideoInfo(QNetworkReply*)));

        // see you in scrapWebPage(QByteArray)

        /*
        qDebug() << videoInfo;
        re = QRegExp("^.*&reason=([^&]+).*$");
        match = re.exactMatch(videoInfo);
        if (match) {
            // report the error in the status bar
            QMainWindow* mainWindow = dynamic_cast<QMainWindow*>(qApp->topLevelWidgets().first());
            QString errorMessage = QUrl::fromEncoded(re.cap(1).toUtf8()).toString().replace("+", " ");
            int indexOfTag = errorMessage.indexOf("<");
            if (indexOfTag != -1) {
                errorMessage = errorMessage.left(indexOfTag);
            }
            if (mainWindow) mainWindow->statusBar()->showMessage(errorMessage);
            emit errorStreamUrl(errorMessage);

        } else
            emit errorStreamUrl("Error parsing video info");
        */

        return;
    }

    QString videoToken = re.cap(1);
    // FIXME proper decode
    videoToken = videoToken.replace("%3D", "=");
    // qDebug() << "token" << videoToken;

    QSettings settings;
    if (settings.value("hd").toBool())
        findHdVideo(videoToken);
    else
        standardVideoUrl(videoToken);

}

void Video::standardVideoUrl(QString videoToken) {
    QUrl videoUrl = QUrl(QString("http://www.youtube.com/get_video?video_id=")
                         .append(videoId)
                         .append("&t=").append(videoToken)
                         .append("&eurl=&el=embedded&ps=default&fmt=18"));
    m_streamUrl = videoUrl;
    m_hd = false;
    emit gotStreamUrl(videoUrl);
}

void Video::hdVideoUrl(QString videoToken) {
    QUrl videoUrl = QUrl(QString("http://www.youtube.com/get_video?video_id=")
                         .append(videoId)
                         .append("&t=").append(videoToken)
                         .append("&eurl=&el=embedded&ps=default&fmt=22"));
    m_streamUrl = videoUrl;
    m_hd = true;
    emit gotStreamUrl(videoUrl);
}

void Video::errorVideoInfo(QNetworkReply *reply) {
    emit errorStreamUrl(tr("Network error: %1 for %2").arg(reply->errorString(), reply->url().toString()));
}

void Video::scrapWebPage(QByteArray data) {

    QString videoHTML = QString::fromUtf8(data);
    QRegExp re(".*, \"t\": \"([^\"]+)\".*");
    bool match = re.exactMatch(videoHTML);

    // on regexp failure, stop and report error
    if (!match || re.numCaptures() < 1) {
        emit errorStreamUrl("Error parsing video page");
        return;
    }

    QString videoToken = re.cap(1);
    // FIXME proper decode
    videoToken = videoToken.replace("%3D", "=");
    // qDebug() << "token" << videoToken;

    QSettings settings;
    if (settings.value("hd").toBool())
        findHdVideo(videoToken);
    else
        standardVideoUrl(videoToken);

}

void Video::findHdVideo(QString videoToken) {

    // we'll need this in gotHeaders()
    this->videoToken = videoToken;

    // try HD: fmt=22
    QUrl videoUrl = QUrl(QString("http://www.youtube.com/get_video?video_id=")
                         .append(videoId)
                         .append("&t=").append(videoToken)
                         .append("&eurl=&el=embedded&ps=default&fmt=22"));

    QObject *reply = The::http()->head(videoUrl);
    connect(reply, SIGNAL(finished(QNetworkReply*)), SLOT(gotHdHeaders(QNetworkReply*)));
    // connect(reply, SIGNAL(error(QNetworkReply*)), SLOT(errorVideoInfo(QNetworkReply*)));

    // see you in gotHeaders()
}

void Video::gotHdHeaders(QNetworkReply* reply) {
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    // qDebug() << "gotHeaders" << statusCode;
    if (statusCode == 200) {
        hdVideoUrl(videoToken);
    } else {
        standardVideoUrl(videoToken);
    }
}
