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
#include "temporary.h"

namespace The {
NetworkAccess* http();
}

namespace {
static const QString jsNameChars = "a-zA-Z0-9\\$_";
}

Video::Video() : m_duration(0),
    m_viewCount(-1),
    m_license(LicenseYouTube),
    definitionCode(0),
    elIndex(0),
    ageGate(false),
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
    QRegExp re(JsFunctions::instance()->videoIdRE());
    if (re.indexIn(m_webpage.toString()) == -1) {
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

    QUrl url;

    if (elIndex == elTypes.size()) {
        // qDebug() << "Trying special embedded el param";
        url = QUrl("http://www.youtube.com/get_video_info");

#if QT_VERSION >= 0x050000
        {
            QUrl &u = url;
            QUrlQuery url;
#endif
            url.addQueryItem("video_id", videoId);
            url.addQueryItem("el", "embedded");
            url.addQueryItem("gl", "US");
            url.addQueryItem("hl", "en");
            url.addQueryItem("eurl", "https://youtube.googleapis.com/v/" + videoId);
            url.addQueryItem("asv", "3");
            url.addQueryItem("sts", "1588");
#if QT_VERSION >= 0x050000
            u.setQuery(url);
        }
#endif

    } else if (elIndex > elTypes.size() - 1) {
        qWarning() << "Cannot get video info";
        loadingStreamUrl = false;
        emit errorStreamUrl("Cannot get video info");
        return;
    } else {
        // qDebug() << "Trying el param:" << elTypes.at(elIndex) << elIndex;
        url = QUrl(QString(
                       "http://www.youtube.com/get_video_info?video_id=%1%2&ps=default&eurl=&gl=US&hl=en"
                       ).arg(videoId, elTypes.at(elIndex)));
    }

    QObject *reply = The::http()->get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(gotVideoInfo(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), SLOT(errorVideoInfo(QNetworkReply*)));

    // see you in gotVideoInfo...
}

void  Video::gotVideoInfo(QByteArray data) {
    QString videoInfo = QString::fromUtf8(data);
    // qDebug() << "videoInfo" << videoInfo;

    // get video token
    QRegExp videoTokeRE(JsFunctions::instance()->videoTokenRE());
    if (videoTokeRE.indexIn(videoInfo) == -1) {
        // qWarning() << "Cannot get token. Trying next el param" << videoInfo << videoTokeRE.pattern();
        // Don't panic! We're gonna try another magic "el" param
        elIndex++;
        getVideoInfo();
        return;
    }

    QString videoToken = videoTokeRE.cap(1);
    // qWarning() << "got token" << videoToken;
    while (videoToken.contains('%'))
        videoToken = QByteArray::fromPercentEncoding(videoToken.toLatin1());
    // qDebug() << "videoToken" << videoToken;
    this->videoToken = videoToken;

    // get fmt_url_map
    QRegExp fmtMapRE(JsFunctions::instance()->videoInfoFmtMapRE());
    if (fmtMapRE.indexIn(videoInfo) == -1) {
        // qWarning() << "Cannot get urlMap. Trying next el param";
        // Don't panic! We're gonna try another magic "el" param
        elIndex++;
        getVideoInfo();
        return;
    }

    // qDebug() << "Got token and urlMap" << elIndex;

    QString fmtUrlMap = fmtMapRE.cap(1);
    // qWarning() << "got fmtUrlMap" << fmtUrlMap;
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

#if QT_VERSION >= 0x050000
                    {
                        QUrl &u = url;
                        QUrlQuery url;
#endif
                        url.addQueryItem("v", videoId);
                        url.addQueryItem("gl", "US");
                        url.addQueryItem("hl", "en");
                        url.addQueryItem("has_verified", "1");
#if QT_VERSION >= 0x050000
                        u.setQuery(url);
                    }
#endif
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

void Video::errorVideoInfo(QNetworkReply *reply) {
    loadingStreamUrl = false;
    emit errorStreamUrl(tr("Network error: %1 for %2").arg(reply->errorString(), reply->url().toString()));
}

void Video::scrapeWebPage(QByteArray data) {
    QString html = QString::fromUtf8(data);
    // qWarning() << html;

    QRegExp ageGateRE(JsFunctions::instance()->ageGateRE());
    if (ageGateRE.indexIn(html) != -1) {
        // qDebug() << "Found ageGate";
        ageGate = true;
        elIndex = 4;
        getVideoInfo();
        return;
    }

    QRegExp fmtMapRE(JsFunctions::instance()->webPageFmtMapRE());
    if (fmtMapRE.indexIn(html) == -1) {
        // qWarning() << "Error parsing video page";
        // emit errorStreamUrl("Error parsing video page");
        // loadingStreamUrl = false;
        elIndex++;
        getVideoInfo();
        return;
    }
    fmtUrlMap = fmtMapRE.cap(1);
    fmtUrlMap.replace("\\u0026", "&");
    // parseFmtUrlMap(fmtUrlMap, true);

#ifdef APP_DASH
    QSettings settings;
    QString definitionName = settings.value("definition", "360p").toString();
    if (definitionName == QLatin1String("1080p") {
        QRegExp dashManifestRe("\"dashmpd\":\\s*\"([^\"]+)\"");
        if (dashManifestRe.indexIn(html) != -1) {
            dashManifestUrl = dashManifestRe.cap(1);
            dashManifestUrl.remove('\\');
            qDebug() << "dashManifestUrl" << dashManifestUrl;
        }
    }
#endif

    QRegExp jsPlayerRe(JsFunctions::instance()->jsPlayerRE());
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

void Video::parseJsPlayer(QByteArray bytes) {
    QString js = QString::fromUtf8(bytes);
    // qWarning() << "jsPlayer" << js;

    // QRegExp funcNameRe("\"signature\"\\w*,\\w*([" + jsNameChars + "]+)");
    QRegExp funcNameRe(JsFunctions::instance()->signatureFunctionNameRE().arg(jsNameChars));

    if (funcNameRe.indexIn(js) == -1) {
        qWarning() << "Cannot capture signature function name" << js;
    } else {
        sigFuncName = funcNameRe.cap(1);
        captureFunction(sigFuncName, js);
        // qWarning() << sigFunctions;
    }

#ifdef APP_DASH
    if (!dashManifestUrl.isEmpty()) {
        QRegExp sigRe("/s/([\\w\\.]+)");
        if (sigRe.indexIn(dashManifestUrl) != -1) {
            qDebug() << "Decrypting signature for dash manifest";
            QString sig = sigRe.cap(1);
            sig = decryptSignature(sig);
            dashManifestUrl.replace(sigRe, "/signature/" + sig);
            qDebug() << dashManifestUrl;

            m_streamUrl = dashManifestUrl;
            this->definitionCode = 37;
            emit gotStreamUrl(m_streamUrl);
            loadingStreamUrl = false;

            /*
            QObject *reply = The::http()->get(QUrl::fromEncoded(dashManifestUrl.toUtf8()));
            connect(reply, SIGNAL(data(QByteArray)), SLOT(parseDashManifest(QByteArray)));
            connect(reply, SIGNAL(error(QNetworkReply*)), SLOT(errorVideoInfo(QNetworkReply*)));
            */

            return;
        }
    }
#endif

    parseFmtUrlMap(fmtUrlMap, true);
}

void Video::parseDashManifest(QByteArray bytes) {
    QFile file(Temporary::filename());
    if (!file.open(QIODevice::WriteOnly))
        qWarning() << file.errorString() << file.fileName();
    QDataStream stream(&file);
    stream.writeRawData(bytes.constData(), bytes.size());

    m_streamUrl = "file://" + file.fileName();
    this->definitionCode = 37;
    emit gotStreamUrl(m_streamUrl);
    loadingStreamUrl = false;
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

QString Video::formattedDuration() const {
    QString format = m_duration > 3600 ? "h:mm:ss" : "m:ss";
    return QTime().addSecs(m_duration).toString(format);
}
