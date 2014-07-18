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

#include "video.h"
#include "networkaccess.h"
#include <QtNetwork>
#include "videodefinition.h"
#include "jsfunctions.h"

namespace The {
NetworkAccess* http();
}

namespace {
    static const QString jsNameChars = "a-zA-Z0-9\\$_";
}

Video::Video() : m_duration(0),
    m_viewCount(-1),
    definitionCode(0),
    elIndex(0),
    ageGate(false),
    m_license(LicenseYouTube),
    loadingStreamUrl(false),
    loadingThumbnail(false)
{ }

Video* Video::clone() {
    Video* cloneVideo = new Video();
    cloneVideo->m_title = m_title;
    cloneVideo->m_description = m_description;
    cloneVideo->m_author = m_author;
    cloneVideo->m_userId = m_userId;
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
        qWarning() << QString("Cannot get video id for %1").arg(m_webpage.toString());
        // emit errorStreamUrl(QString("Cannot get video id for %1").arg(m_webpage.toString()));
        // loadingStreamUrl = false;
        return;
    }
    videoId = re.cap(1);
}

void Video::loadThumbnail() {
    if (m_thumbnailUrl.isEmpty() || loadingThumbnail) return;
    loadingThumbnail = true;
    QObject *reply = The::http()->get(m_thumbnailUrl);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(setThumbnail(QByteArray)));
}

void Video::setThumbnail(QByteArray bytes) {
    loadingThumbnail = false;
    m_thumbnail.loadFromData(bytes);
    if (m_thumbnail.width() > 160)
        m_thumbnail = m_thumbnail.scaledToWidth(160, Qt::SmoothTransformation);
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
    elIndex = 0;
    ageGate = false;

    getVideoInfo();
}

void  Video::getVideoInfo() {
    static const QStringList elTypes = QStringList() << "&el=embedded" << "&el=detailpage" << "&el=vevo" << "";

    QUrl videoInfoUrl;

    if (elIndex == elTypes.size()) {
        // qDebug() << "Trying special embedded el param";
        videoInfoUrl = QUrl("http://www.youtube.com/get_video_info");
        videoInfoUrl.addQueryItem("video_id", videoId);
        videoInfoUrl.addQueryItem("el", "embedded");
        videoInfoUrl.addQueryItem("gl", "US");
        videoInfoUrl.addQueryItem("hl", "en");
        videoInfoUrl.addQueryItem("eurl", "https://youtube.googleapis.com/v/" + videoId);
        videoInfoUrl.addQueryItem("asv", "3");
        videoInfoUrl.addQueryItem("sts", "1588");
    } else if (elIndex > elTypes.size() - 1) {
        qWarning() << "Cannot get video info";
        loadingStreamUrl = false;
        emit errorStreamUrl("Cannot get video info");
        return;
    } else {
        // qDebug() << "Trying el param:" << elTypes.at(elIndex) << elIndex;
        videoInfoUrl = QUrl(QString(
                                "http://www.youtube.com/get_video_info?video_id=%1%2&ps=default&eurl=&gl=US&hl=en"
                                ).arg(videoId, elTypes.at(elIndex)));
    }

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
        // qDebug() << "Cannot get token. Trying next el param";
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
        // qDebug() << "Cannot get urlMap. Trying next el param";
        // Don't panic! We're gonna try another magic "el" param
        elIndex++;
        getVideoInfo();
        return;
    }

    // qDebug() << "Got token and urlMap" << elIndex;

    QString fmtUrlMap = re.cap(1);
    fmtUrlMap = QByteArray::fromPercentEncoding(fmtUrlMap.toUtf8());
    parseFmtUrlMap(fmtUrlMap);
}

void Video::parseFmtUrlMap(const QString &fmtUrlMap, bool fromWebPage) {
    QSettings settings;
    QString definitionName = settings.value("definition", "360p").toString();
    int definitionCode = VideoDefinition::getDefinitionCode(definitionName);

    // qDebug() << "fmtUrlMap" << fmtUrlMap;
    QStringList formatUrls = fmtUrlMap.split(',', QString::SkipEmptyParts);
    QHash<int, QString> urlMap;
    foreach(QString formatUrl, formatUrls) {
        // qDebug() << "formatUrl" << formatUrl;
        QStringList urlParams = formatUrl.split('&', QString::SkipEmptyParts);
        // qDebug() << "urlParams" << urlParams;

        int format = -1;
        QString url;
        QString sig;
        foreach(QString urlParam, urlParams) {
            // qWarning() << urlParam;
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
            } else if (urlParam.startsWith("s=")) {
                if (fromWebPage || ageGate) {
                    int separator = urlParam.indexOf("=");
                    sig = urlParam.mid(separator + 1);
                    sig = QByteArray::fromPercentEncoding(sig.toUtf8());
                    if (ageGate)
                        sig = JsFunctions::instance()->decryptAgeSignature(sig);
                    else {
                        sig = decryptSignature(sig);
                        if (sig.isEmpty())
                            sig = JsFunctions::instance()->decryptSignature(sig);
                    }
                } else {
                    // qDebug() << "Loading webpage";
                    QUrl url("http://www.youtube.com/watch");
                    url.addQueryItem("v", videoId);
                    url.addQueryItem("gl", "US");
                    url.addQueryItem("hl", "en");
                    url.addQueryItem("has_verified", "1");
                    QObject *reply = The::http()->get(url);
                    connect(reply, SIGNAL(data(QByteArray)), SLOT(scrapeWebPage(QByteArray)));
                    connect(reply, SIGNAL(error(QNetworkReply*)), SLOT(errorVideoInfo(QNetworkReply*)));
                    // see you in scrapWebPage(QByteArray)
                    return;
                }
            }
        }
        if (format == -1 || url.isNull()) continue;

        url += "&signature=" + sig;

        if (!url.contains("ratebypass"))
            url += "&ratebypass=yes";

        // qWarning() << url;

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
    QString html = QString::fromUtf8(data);
    // qWarning() << html;

    if (html.contains("player-age-gate-content\"")) {
        // qDebug() << "Found ageGate";
        ageGate = true;
        elIndex = 4;
        getVideoInfo();
        return;
    }

    QRegExp re(".*\"url_encoded_fmt_stream_map\":\\s+\"([^\"]+)\".*");
    bool match = re.exactMatch(html);
    // on regexp failure, stop and report error
    if (!match || re.numCaptures() < 1) {
        qWarning() << "Error parsing video page";
        // emit errorStreamUrl("Error parsing video page");
        // loadingStreamUrl = false;
        elIndex++;
        getVideoInfo();
        return;
    }
    fmtUrlMap = re.cap(1);
    fmtUrlMap.replace("\\u0026", "&");
    // parseFmtUrlMap(fmtUrlMap, true);

    QRegExp jsPlayerRe("\"assets\":.+\"js\":\\s*\"([^\"]+)\"");
    if (jsPlayerRe.indexIn(html) != -1) {
        QString jsPlayerUrl = jsPlayerRe.cap(1);
        jsPlayerUrl.remove('\\');
        jsPlayerUrl = "http:" + jsPlayerUrl;
        // qDebug() << "jsPlayerUrl" << jsPlayerUrl;
        /*
        QRegExp jsPlayerIdRe("-(.+)\\.js");
        jsPlayerIdRe.indexIn(jsPlayerUrl);
        QString jsPlayerId = jsPlayerRe.cap(1);
        */
        QObject *reply = The::http()->get(jsPlayerUrl);
        connect(reply, SIGNAL(data(QByteArray)), SLOT(parseJsPlayer(QByteArray)));
        connect(reply, SIGNAL(error(QNetworkReply*)), SLOT(errorVideoInfo(QNetworkReply*)));
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

void Video::parseJsPlayer(QByteArray bytes) {
    QString js = QString::fromUtf8(bytes);
    // qWarning() << "jsPlayer" << js;
    QRegExp funcNameRe("signature=([" + jsNameChars + "]+)");
    if (funcNameRe.indexIn(js) == -1) {
        qWarning() << "Cannot capture signature function name";
    } else {
        sigFuncName = funcNameRe.cap(1);
        captureFunction(sigFuncName, js);
        // qWarning() << sigFunctions;
    }
    parseFmtUrlMap(fmtUrlMap, true);
}

void Video::captureFunction(const QString &name, const QString &js) {
    QRegExp funcRe("function\\s+" + QRegExp::escape(name) + "\\s*\\([" + jsNameChars + ",\\s]*\\)\\s*\\{[^\\}]+\\}");
    if (funcRe.indexIn(js) == -1) {
        qWarning() << "Cannot capture function" << name;
        return;
    }
    QString func = funcRe.cap(0);
    sigFunctions.insert(name, func);

    // capture inner functions
    QRegExp invokedFuncRe("[\\s=;\\(]([" + jsNameChars + "]+)\\s*\\([" + jsNameChars + ",\\s]+\\)");
    int pos = name.length() + 9;
    while ((pos = invokedFuncRe.indexIn(func, pos)) != -1) {
        QString funcName = invokedFuncRe.cap(1);
        if (!sigFunctions.contains(funcName))
            captureFunction(funcName, js);
        pos += invokedFuncRe.matchedLength();
    }

    // capture referenced objects
    QRegExp objRe("[\\s=;\\(]([" + jsNameChars + "]+)\\.[" + jsNameChars + "]+");
    pos = name.length() + 9;
    while ((pos = objRe.indexIn(func, pos)) != -1) {
        QString objName = objRe.cap(1);
        if (!sigObjects.contains(objName))
            captureObject(objName, js);
        pos += objRe.matchedLength();
    }
}

void Video::captureObject(const QString &name, const QString &js) {
    QRegExp re("var\\s+" + QRegExp::escape(name) + "\\s*=\\s*\\{.+\\}\\s*;");
    re.setMinimal(true);
    if (re.indexIn(js) == -1) {
        qWarning() << "Cannot capture object" << name;
        return;
    }
    QString obj = re.cap(0);
    sigObjects.insert(name, obj);
}

QString Video::decryptSignature(const QString &s) {
    if (sigFuncName.isEmpty()) return QString();
    QScriptEngine engine;
    foreach (QString f, sigObjects.values()) {
        QScriptValue value = engine.evaluate(f);
        if (value.isError())
            qWarning() << "Error in" << f << value.toString();
    }
    foreach (QString f, sigFunctions.values()) {
        QScriptValue value = engine.evaluate(f);
        if (value.isError())
            qWarning() << "Error in" << f << value.toString();
    }
    QString js = sigFuncName + "('" + s + "');";
    QScriptValue value = engine.evaluate(js);
    if (value.isUndefined()) {
        qWarning() << "Undefined result for" << js;
        return QString();
    }
    if (value.isError()) {
        qWarning() << "Error in" << js << value.toString();
        return QString();
    }
    return value.toString();
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
