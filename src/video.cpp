#include "video.h"
#include "networkaccess.h"
#include <QtNetwork>
#include "videodefinition.h"

namespace The {
    NetworkAccess* http();
}

Video::Video() : m_duration(0),
m_viewCount(-1),
definitionCode(0),
elIndex(0),
loadingStreamUrl(false)
{ }

Video* Video::clone() {
    Video* cloneVideo = new Video();
    cloneVideo->m_title = m_title;
    cloneVideo->m_description = m_description;
    cloneVideo->m_author = m_author;
    cloneVideo->m_authorUri = m_authorUri;
    cloneVideo->m_webpage = m_webpage;
    cloneVideo->m_streamUrl = m_streamUrl;
    cloneVideo->m_thumbnail = m_thumbnail;
    cloneVideo->m_thumbnailUrl = m_thumbnailUrl;
    cloneVideo->m_mediumThumbnailUrl = m_mediumThumbnailUrl;
    cloneVideo->m_duration = m_duration;
    cloneVideo->m_published = m_published;
    cloneVideo->m_viewCount = m_viewCount;
    cloneVideo->videoId = videoId;
    cloneVideo->videoToken = videoToken;
    cloneVideo->definitionCode = definitionCode;
    return cloneVideo;
}

void Video::setWebpage(QUrl webpage) {
    m_webpage = webpage;

    // Get Video ID
    // youtube-dl line 428
    // QRegExp re("^((?:http://)?(?:\\w+\\.)?youtube\\.com/(?:(?:v/)|(?:(?:watch(?:\\.php)?)?\\?(?:.+&)?v=)))?([0-9A-Za-z_-]+)(?(1).+)?$");
    QRegExp re("^https?://www\\.youtube\\.com/watch\\?v=([0-9A-Za-z_-]+).*");
    bool match = re.exactMatch(m_webpage.toString());
    if (!match || re.numCaptures() < 1) {
        qDebug() << QString("Cannot get video id for %1").arg(m_webpage.toString());
        // emit errorStreamUrl(QString("Cannot get video id for %1").arg(m_webpage.toString()));
        // loadingStreamUrl = false;
        return;
    }
    videoId = re.cap(1);
}

void Video::loadThumbnail() {
    QObject *reply = The::http()->get(m_thumbnailUrl);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(setThumbnail(QByteArray)));
}

void Video::setThumbnail(QByteArray bytes) {
    m_thumbnail.loadFromData(bytes);
    emit gotThumbnail();
}

void Video::loadMediumThumbnail() {
    if (m_mediumThumbnailUrl.isEmpty()) return;
    QObject *reply = The::http()->get(m_mediumThumbnailUrl);
    connect(reply, SIGNAL(data(QByteArray)), SIGNAL(gotMediumThumbnail(QByteArray)));
}

void Video::loadStreamUrl() {
    if (loadingStreamUrl) {
        qDebug() << "Already loading stream URL for" << this->title();
        return;
    }
    loadingStreamUrl = true;

    // https://develop.participatoryculture.org/trac/democracy/browser/trunk/tv/portable/flashscraper.py
    getVideoInfo();
}

void  Video::getVideoInfo() {
    static const QStringList elTypes = QStringList() << "&el=embedded" << "&el=vevo" << "&el=detailpage" << "";

    if (elIndex > elTypes.size() - 1) {
        loadingStreamUrl = false;
        emit errorStreamUrl("Cannot get video info");
        /*
        // Don't panic! We have a plan B.
        // get the youtube video webpage
        qDebug() << "Scraping" << webpage().toString();
        QObject *reply = The::http()->get(webpage().toString());
        connect(reply, SIGNAL(data(QByteArray)), SLOT(scrapeWebPage(QByteArray)));
        connect(reply, SIGNAL(error(QNetworkReply*)), SLOT(errorVideoInfo(QNetworkReply*)));
        // see you in scrapWebPage(QByteArray)
        */
        return;
    }

    // Get Video Token
    QUrl videoInfoUrl = QUrl(QString(
            "http://www.youtube.com/get_video_info?video_id=%1%2&ps=default&eurl=&gl=US&hl=en"
            ).arg(videoId, elTypes.at(elIndex)));

    QObject *reply = The::http()->get(videoInfoUrl);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(gotVideoInfo(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), SLOT(errorVideoInfo(QNetworkReply*)));

    // see you in gotVideoInfo...

}

void  Video::gotVideoInfo(QByteArray data) {
    QString videoInfo = QString::fromUtf8(data);

    // qDebug() << "videoInfo" << videoInfo;

    // get video token
    QRegExp re = QRegExp("^.*&token=([^&]+).*$");
    bool match = re.exactMatch(videoInfo);
    // handle regexp failure
    if (!match || re.numCaptures() < 1) {
        // Don't panic! We're gonna try another magic "el" param
        elIndex++;
        getVideoInfo();
        return;
    }
    QString videoToken = re.cap(1);
    while (videoToken.contains('%'))
        videoToken = QByteArray::fromPercentEncoding(videoToken.toAscii());
    // qDebug() << "videoToken" << videoToken;
    this->videoToken = videoToken;

    // get fmt_url_map
    re = QRegExp("^.*&url_encoded_fmt_stream_map=([^&]+).*$");
    match = re.exactMatch(videoInfo);
    // handle regexp failure
    if (!match || re.numCaptures() < 1) {
        // Don't panic! We're gonna try another magic "el" param
        elIndex++;
        getVideoInfo();
        return;
    }

    QString fmtUrlMap = re.cap(1);
    fmtUrlMap = QByteArray::fromPercentEncoding(fmtUrlMap.toUtf8());

    QSettings settings;
    QString definitionName = settings.value("definition").toString();
    int definitionCode = VideoDefinition::getDefinitionCode(definitionName);

    // qDebug() << "fmtUrlMap" << fmtUrlMap;
    QStringList formatUrls = fmtUrlMap.split(",", QString::SkipEmptyParts);
    QHash<int, QString> urlMap;
    foreach(QString formatUrl, formatUrls) {
        // qDebug() << "formatUrl" << formatUrl;
        QStringList urlParams = formatUrl.split("&", QString::SkipEmptyParts);
        // qDebug() << "urlParams" << urlParams;

        int format = -1;
        QString url;
        QString sig;
        foreach(QString urlParam, urlParams) {
            if (urlParam.startsWith("itag=")) {
                int separator = urlParam.indexOf("=");
                format = urlParam.mid(separator + 1).toInt();
            } else if (urlParam.startsWith("url=")) {
                int separator = urlParam.indexOf("=");
                url = urlParam.mid(separator + 1);
                url = QByteArray::fromPercentEncoding(url.toUtf8());
            } else if (urlParam.startsWith("sig=")) {
                int separator = urlParam.indexOf("=");
                sig = urlParam.mid(separator + 1);
                sig = QByteArray::fromPercentEncoding(sig.toUtf8());
            }
        }
        if (format == -1 || url.isNull()) continue;

        url += "&signature=" + sig;

        if (format == definitionCode) {
            qDebug() << "Found format" << definitionCode;
            QUrl videoUrl = QUrl::fromEncoded(url.toUtf8(), QUrl::StrictMode);
            m_streamUrl = videoUrl;
            this->definitionCode = definitionCode;
            emit gotStreamUrl(videoUrl);
            loadingStreamUrl = false;
            return;
        }

        urlMap.insert(format, url);
    }

    QList<int> definitionCodes = VideoDefinition::getDefinitionCodes();
    int currentIndex = definitionCodes.indexOf(definitionCode);
    int previousIndex = 0;
    while (currentIndex >= 0) {
        previousIndex = currentIndex - 1;
        if (previousIndex < 0) previousIndex = 0;
        int definitionCode = definitionCodes.at(previousIndex);
        if (urlMap.contains(definitionCode)) {
            qDebug() << "Found format" << definitionCode;
            QString url = urlMap.value(definitionCode);
            QUrl videoUrl = QUrl::fromEncoded(url.toUtf8(), QUrl::StrictMode);
            m_streamUrl = videoUrl;
            this->definitionCode = definitionCode;
            emit gotStreamUrl(videoUrl);
            loadingStreamUrl = false;
            return;
        }
        currentIndex--;
    }

    emit errorStreamUrl(tr("Cannot get video stream for %1").arg(m_webpage.toString()));

}

void Video::foundVideoUrl(QString videoToken, int definitionCode) {
    // qDebug() << "foundVideoUrl" << videoToken << definitionCode;

    QUrl videoUrl = QUrl(QString(
            "http://www.youtube.com/get_video?video_id=%1&t=%2&eurl=&el=&ps=&asv=&fmt=%3"
            ).arg(videoId, videoToken, QString::number(definitionCode)));

    m_streamUrl = videoUrl;
    loadingStreamUrl = false;
    emit gotStreamUrl(videoUrl);
}

void Video::errorVideoInfo(QNetworkReply *reply) {
    loadingStreamUrl = false;
    emit errorStreamUrl(tr("Network error: %1 for %2").arg(reply->errorString(), reply->url().toString()));
}

void Video::scrapeWebPage(QByteArray data) {

    QString videoHTML = QString::fromUtf8(data);
    QRegExp re(".*, \"t\": \"([^\"]+)\".*");
    bool match = re.exactMatch(videoHTML);

    // on regexp failure, stop and report error
    if (!match || re.numCaptures() < 1) {
        emit errorStreamUrl("Error parsing video page");
        loadingStreamUrl = false;
        return;
    }

    QString videoToken = re.cap(1);
    // FIXME proper decode
    videoToken = videoToken.replace("%3D", "=");

    // we'll need this in gotHeadHeaders()
    this->videoToken = videoToken;

    // qDebug() << "token" << videoToken;

    QSettings settings;
    QString definitionName = settings.value("definition").toString();
    int definitionCode = VideoDefinition::getDefinitionCode(definitionName);
    if (definitionCode == 18) {
        // This is assumed always available
        foundVideoUrl(videoToken, 18);
    } else {
        findVideoUrl(definitionCode);
    }

}

void Video::gotHeadHeaders(QNetworkReply* reply) {
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    // qDebug() << "gotHeaders" << statusCode;
    if (statusCode == 200) {
        foundVideoUrl(videoToken, definitionCode);
    } else {

        // try next (lower quality) definition
        /*
        QStringList definitionNames = VideoDefinition::getDefinitionNames();
        int currentIndex = definitionNames.indexOf(currentDefinition);
        int previousIndex = 0;
        if (currentIndex > 0) {
            previousIndex = currentIndex - 1;
        }
        if (previousIndex > 0) {
            QString nextDefinitionName = definitionNames.at(previousIndex);
            findVideoUrl(nextDefinitionName);
        } else {
            foundVideoUrl(videoToken, 18);
        }*/


        QList<int> definitionCodes = VideoDefinition::getDefinitionCodes();
        int currentIndex = definitionCodes.indexOf(definitionCode);
        int previousIndex = 0;
        if (currentIndex > 0) {
            previousIndex = currentIndex - 1;
            int definitionCode = definitionCodes.at(previousIndex);
            if (definitionCode == 18) {
                // This is assumed always available
                foundVideoUrl(videoToken, 18);
            } else {
                findVideoUrl(definitionCode);
            }

        } else {
            foundVideoUrl(videoToken, 18);
        }

    }
}

void Video::findVideoUrl(int definitionCode) {
    this->definitionCode = definitionCode;

    QUrl videoUrl = QUrl(QString(
            "http://www.youtube.com/get_video?video_id=%1&t=%2&eurl=&el=&ps=&asv=&fmt=%3"
            ).arg(videoId, videoToken, QString::number(definitionCode)));

    QObject *reply = The::http()->head(videoUrl);
    connect(reply, SIGNAL(finished(QNetworkReply*)), SLOT(gotHeadHeaders(QNetworkReply*)));
    // connect(reply, SIGNAL(error(QNetworkReply*)), SLOT(errorVideoInfo(QNetworkReply*)));

    // see you in gotHeadHeaders()

}


QString Video::formattedDuration() const {
    QString format = m_duration > 3600 ? "h:mm:ss" : "m:ss";
    return QTime().addSecs(m_duration).toString(format);
}
