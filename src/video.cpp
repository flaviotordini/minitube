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
#include "http.h"
#include "httputils.h"
#include "videodefinition.h"
#include "jsfunctions.h"
#include "temporary.h"
#include "datautils.h"

#include <QtNetwork>
#include <QJSEngine>
#include <QJSValue>

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
    loadingThumbnail(false) {
}

Video* Video::clone() {
    Video* cloneVideo = new Video();
    cloneVideo->m_title = m_title;
    cloneVideo->m_description = m_description;
    cloneVideo->m_channelTitle = m_channelTitle;
    cloneVideo->m_channelId = m_channelId;
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

const QString &Video::webpage() {
    if (m_webpage.isEmpty() && !videoId.isEmpty())
        m_webpage.append("https://www.youtube.com/watch?v=").append(videoId);
    return m_webpage;
}

void Video::setWebpage(const QString &value) {
    m_webpage = value;

    // Get Video ID
    if (videoId.isEmpty()) {
        QRegExp re(JsFunctions::instance()->videoIdRE());
        if (re.indexIn(m_webpage) == -1) {
            qWarning() << QString("Cannot get video id for %1").arg(m_webpage);
            // emit errorStreamUrl(QString("Cannot get video id for %1").arg(m_webpage.toString()));
            // loadingStreamUrl = false;
            return;
        }
        videoId = re.cap(1);
    }
}

void Video::loadThumbnail() {
    if (m_thumbnailUrl.isEmpty() || loadingThumbnail) return;
    loadingThumbnail = true;
    QObject *reply = HttpUtils::yt().get(m_thumbnailUrl);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(setThumbnail(QByteArray)));
}

void Video::setThumbnail(const QByteArray &bytes) {
    loadingThumbnail = false;
    qreal ratio = qApp->devicePixelRatio();
    m_thumbnail = QPixmap();
    m_thumbnail.loadFromData(bytes);
    m_thumbnail.setDevicePixelRatio(ratio);
    const int thumbWidth = 160 * ratio;
    if (m_thumbnail.width() > thumbWidth)
        m_thumbnail = m_thumbnail.scaledToWidth(thumbWidth, Qt::SmoothTransformation);
    emit gotThumbnail();
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
        url = QUrl("https://www.youtube.com/get_video_info");
        QUrlQuery q;
        q.addQueryItem("video_id", videoId);
        q.addQueryItem("el", "embedded");
        q.addQueryItem("gl", "US");
        q.addQueryItem("hl", "en");
        q.addQueryItem("eurl", "https://youtube.googleapis.com/v/" + videoId);
        q.addQueryItem("asv", "3");
        q.addQueryItem("sts", "1588");
        url.setQuery(q);
    } else if (elIndex > elTypes.size() - 1) {
        qWarning() << "Cannot get video info";
        loadingStreamUrl = false;
        emit errorStreamUrl("Cannot get video info");
        return;
    } else {
        // qDebug() << "Trying el param:" << elTypes.at(elIndex) << elIndex;
        url = QUrl(QString(
                       "https://www.youtube.com/get_video_info?video_id=%1%2&ps=default&eurl=&gl=US&hl=en"
                       ).arg(videoId, elTypes.at(elIndex)));
    }

    QObject *reply = HttpUtils::yt().get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(gotVideoInfo(QByteArray)));
    connect(reply, SIGNAL(error(QString)), SLOT(errorVideoInfo(QString)));

    // see you in gotVideoInfo...
}

void  Video::gotVideoInfo(const QByteArray &bytes) {
    QString videoInfo = QString::fromUtf8(bytes);
    // qDebug() << "videoInfo" << videoInfo;

    // get video token
    QRegExp videoTokeRE(JsFunctions::instance()->videoTokenRE());
    if (videoTokeRE.indexIn(videoInfo) == -1) {
        qDebug() << "Cannot get token. Trying next el param" << videoInfo << videoTokeRE.pattern();
        // Don't panic! We're gonna try another magic "el" param
        elIndex++;
        getVideoInfo();
        return;
    }

    QString videoToken = videoTokeRE.cap(1);
    qDebug() << "got token" << videoToken;
    while (videoToken.contains('%'))
        videoToken = QByteArray::fromPercentEncoding(videoToken.toLatin1());
    qDebug() << "videoToken" << videoToken;
    this->videoToken = videoToken;

    // get fmt_url_map
    QRegExp fmtMapRE(JsFunctions::instance()->videoInfoFmtMapRE());
    if (fmtMapRE.indexIn(videoInfo) == -1) {
        qDebug() << "Cannot get urlMap. Trying next el param";
        // Don't panic! We're gonna try another magic "el" param
        elIndex++;
        getVideoInfo();
        return;
    }

    QString fmtUrlMap = fmtMapRE.cap(1);
    // qDebug() << "got fmtUrlMap" << fmtUrlMap;
    fmtUrlMap = QByteArray::fromPercentEncoding(fmtUrlMap.toUtf8());

    qDebug() << "Got token and urlMap" << elIndex << videoToken << fmtUrlMap;
    parseFmtUrlMap(fmtUrlMap);
}

void Video::parseFmtUrlMap(const QString &fmtUrlMap, bool fromWebPage) {
    const QString definitionName = QSettings().value("definition", "360p").toString();
    const VideoDefinition& definition = VideoDefinition::getDefinitionFor(definitionName);

    qDebug() << "fmtUrlMap" << fmtUrlMap;
    const QVector<QStringRef> formatUrls = fmtUrlMap.splitRef(',', QString::SkipEmptyParts);
    QHash<int, QString> urlMap;
    foreach(const QStringRef &formatUrl, formatUrls) {
        // qDebug() << "formatUrl" << formatUrl;
        const QVector<QStringRef> urlParams = formatUrl.split('&', QString::SkipEmptyParts);
        // qDebug() << "urlParams" << urlParams;

        int format = -1;
        QString url;
        QString sig;
        foreach(const QStringRef &urlParam, urlParams) {
            // qWarning() << urlParam;
            if (urlParam.startsWith(QLatin1String("itag="))) {
                int separator = urlParam.indexOf('=');
                format = urlParam.mid(separator + 1).toInt();
            } else if (urlParam.startsWith(QLatin1String("url="))) {
                int separator = urlParam.indexOf('=');
                url = QByteArray::fromPercentEncoding(urlParam.mid(separator + 1).toUtf8());
            } else if (urlParam.startsWith(QLatin1String("sig="))) {
                int separator = urlParam.indexOf('=');
                sig = QByteArray::fromPercentEncoding(urlParam.mid(separator + 1).toUtf8());
            } else if (urlParam.startsWith(QLatin1String("s="))) {
                if (fromWebPage || ageGate) {
                    int separator = urlParam.indexOf('=');
                    sig = QByteArray::fromPercentEncoding(urlParam.mid(separator + 1).toUtf8());
                    if (ageGate)
                        sig = JsFunctions::instance()->decryptAgeSignature(sig);
                    else {
                        sig = decryptSignature(sig);
                        if (sig.isEmpty())
                            sig = JsFunctions::instance()->decryptSignature(sig);
                    }
                } else {

                    QUrl url("https://www.youtube.com/watch");
                    QUrlQuery q;
                    q.addQueryItem("v", videoId);
                    q.addQueryItem("gl", "US");
                    q.addQueryItem("hl", "en");
                    q.addQueryItem("has_verified", "1");
                    url.setQuery(q);
                    qDebug() << "Loading webpage" << url;
                    QObject *reply = HttpUtils::yt().get(url);
                    connect(reply, SIGNAL(data(QByteArray)), SLOT(scrapeWebPage(QByteArray)));
                    connect(reply, SIGNAL(error(QString)), SLOT(errorVideoInfo(QString)));
                    // see you in scrapWebPage(QByteArray)
                    return;
                }
            }
        }
        if (format == -1 || url.isNull()) continue;

        url += QLatin1String("&signature=") + sig;

        if (!url.contains(QLatin1String("ratebypass")))
            url += QLatin1String("&ratebypass=yes");

        qDebug() << url;

        if (format == definition.getCode()) {
            qDebug() << "Found format" << definitionCode;
            saveDefinitionForUrl(url, definition);
            return;
        }

        urlMap.insert(format, url);
    }

    const QList<VideoDefinition>& definitions = VideoDefinition::getDefinitions();
    int previousIndex = std::max(definitions.indexOf(definition) - 1, 0);
    for (; previousIndex >= 0; previousIndex--) {
        const VideoDefinition& previousDefinition = definitions.at(previousIndex);
        if (urlMap.contains(previousDefinition.getCode())) {
            // qDebug() << "Found format" << definitionCode;
            saveDefinitionForUrl(urlMap.value(previousDefinition.getCode()),
                                 previousDefinition);
            return;
        }
    }

    emit errorStreamUrl(tr("Cannot get video stream for %1").arg(m_webpage));
}

void Video::errorVideoInfo(const QString &message) {
    loadingStreamUrl = false;
    emit errorStreamUrl(message);
}

void Video::scrapeWebPage(const QByteArray &bytes) {
    QString html = QString::fromUtf8(bytes);

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
        qWarning() << "Error parsing video page";
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
    if (definitionName == QLatin1String("1080p")) {
        QRegExp dashManifestRe("\"dashmpd\":\\s*\"([^\"]+)\"");
        if (dashManifestRe.indexIn(html) != -1) {
            dashManifestUrl = dashManifestRe.cap(1);
            dashManifestUrl.remove('\\');
            qDebug() << "dashManifestUrl" << dashManifestUrl;
        } else {
            qWarning() << "DASH manifest not found in webpage";
            if (dashManifestRe.indexIn(fmtUrlMap) != -1) {
                dashManifestUrl = dashManifestRe.cap(1);
                dashManifestUrl.remove('\\');
                qDebug() << "dashManifestUrl" << dashManifestUrl;
            } else qWarning() << "DASH manifest not found in fmtUrlMap" << fmtUrlMap;
        }
    }
#endif

    QRegExp jsPlayerRe(JsFunctions::instance()->jsPlayerRE());
    if (jsPlayerRe.indexIn(html) != -1) {
        QString jsPlayerUrl = jsPlayerRe.cap(1);
        jsPlayerUrl.remove('\\');
        if (jsPlayerUrl.startsWith(QLatin1String("//"))) {
            jsPlayerUrl = QLatin1String("https:") + jsPlayerUrl;
        } else if (jsPlayerUrl.startsWith("/")) {
            jsPlayerUrl = QLatin1String("https://youtube.com") + jsPlayerUrl;
        }
        // qDebug() << "jsPlayerUrl" << jsPlayerUrl;
        /*
                    QRegExp jsPlayerIdRe("-(.+)\\.js");
                    jsPlayerIdRe.indexIn(jsPlayerUrl);
                    QString jsPlayerId = jsPlayerRe.cap(1);
                    */
        QObject *reply = HttpUtils::yt().get(jsPlayerUrl);
        connect(reply, SIGNAL(data(QByteArray)), SLOT(parseJsPlayer(QByteArray)));
        connect(reply, SIGNAL(error(QString)), SLOT(errorVideoInfo(QString)));
    }
}

void Video::parseJsPlayer(const QByteArray &bytes) {
    QString js = QString::fromUtf8(bytes);
    jsPlayer = js;
    // qDebug() << "jsPlayer" << js;

    // QRegExp funcNameRe("[\"']signature[\"']\\s*,\\s*([" + jsNameChars + "]+)\\(");
    QRegExp funcNameRe(JsFunctions::instance()->signatureFunctionNameRE().arg(jsNameChars));

    if (funcNameRe.indexIn(js) == -1) {
        qWarning() << "Cannot capture signature function name" << js;
    } else {
        sigFuncName = funcNameRe.cap(1);
        captureFunction(sigFuncName, js);
        // qWarning() << sigFunctions << sigObjects;
    }

#ifdef APP_DASH
    if (!dashManifestUrl.isEmpty()) {
        QRegExp sigRe("/s/([\\w\\.]+)");
        if (sigRe.indexIn(dashManifestUrl) != -1) {
            qDebug() << "Decrypting signature for dash manifest";
            QString sig = sigRe.cap(1);
            sig = decryptSignature(sig);
            dashManifestUrl.replace(sigRe, "/signature/" + sig);
            qWarning() << "dash manifest" << dashManifestUrl;

            if (true) {
                // let phonon play the manifest
                m_streamUrl = dashManifestUrl;
                this->definitionCode = 37;
                emit gotStreamUrl(m_streamUrl);
                loadingStreamUrl = false;
            } else {
                // download the manifest
                QObject *reply = HttpUtils::yt().get(QUrl::fromEncoded(dashManifestUrl.toUtf8()));
                connect(reply, SIGNAL(data(QByteArray)), SLOT(parseDashManifest(QByteArray)));
                connect(reply, SIGNAL(error(QString)), SLOT(errorVideoInfo(QString)));
            }

            return;
        }
    }
#endif

    parseFmtUrlMap(fmtUrlMap, true);
}

void Video::parseDashManifest(const QByteArray &bytes) {
    QFile file(Temporary::filename() + ".mpd");
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
    qDebug() << __PRETTY_FUNCTION__ << name;
    const QString argsAndBody = QLatin1String("\\s*\\([") + jsNameChars +
            QLatin1String(",\\s]*\\)\\s*\\{[^\\}]+\\}");
    QString func;
    QRegExp funcRe(QLatin1String("function\\s+") + QRegExp::escape(name) + argsAndBody);
    if (funcRe.indexIn(js) != -1) {
        func = funcRe.cap(0);
    } else {
        // try var foo = function(bar) { };
        funcRe = QRegExp(QLatin1String("var\\s+") + QRegExp::escape(name) +
                         QLatin1String("\\s*=\\s*function") + argsAndBody);
        if (funcRe.indexIn(js) != -1) {
            func = funcRe.cap(0);
        } else {
            // try ,gr= function(bar) { };
            funcRe = QRegExp(QLatin1String("[,\\s;}\\.\\)](") + QRegExp::escape(name) +
                             QLatin1String("\\s*=\\s*function") + argsAndBody + ")");
            if (funcRe.indexIn(js) != -1) {
                func = funcRe.cap(1);
            } else {
                qWarning() << "Cannot capture function" << name;
                return;
            }
        }
    }
    sigFunctions.insert(name, func);

    // capture inner functions
    QRegExp invokedFuncRe(QLatin1String("[\\s=;\\(]([") + jsNameChars + QLatin1String("]+)\\s*\\([") +
                          jsNameChars + QLatin1String(",\\s]+\\)"));
    int pos = name.length() + 9;
    while ((pos = invokedFuncRe.indexIn(func, pos)) != -1) {
        QString funcName = invokedFuncRe.cap(1);
        if (!sigFunctions.contains(funcName))
            captureFunction(funcName, js);
        pos += invokedFuncRe.matchedLength();
    }

    // capture referenced objects
    QRegExp objRe(QLatin1String("[\\s=;\\(]([") + jsNameChars + QLatin1String("]+)\\.[") +
                  jsNameChars + QLatin1String("]+"));
    pos = name.length() + 9;
    while ((pos = objRe.indexIn(func, pos)) != -1) {
        QString objName = objRe.cap(1);
        if (!sigObjects.contains(objName))
            captureObject(objName, js);
        pos += objRe.matchedLength();
    }
}

void Video::captureObject(const QString &name, const QString &js) {
    QRegExp re(QLatin1String("var\\s+") + QRegExp::escape(name) + QLatin1String("\\s*=\\s*\\{.*\\}\\s*;"));
    re.setMinimal(true);
    if (re.indexIn(js) == -1) {
        qWarning() << "Cannot capture object" << name;
        return;
    }
    QString obj = re.cap(0);
    sigObjects.insert(name, obj);
}

QString Video::decryptSignature(const QString &s) {
    qDebug() << "decryptSignature" << sigFuncName << sigFunctions << sigObjects;
    if (sigFuncName.isEmpty()) return QString();
    QJSEngine engine;
    foreach (const QString &f, sigObjects.values()) {
        QJSValue value = engine.evaluate(f);
        if (value.isError())
            qWarning() << "Error in" << f << value.toString();
    }
    foreach (const QString &f, sigFunctions.values()) {
        QJSValue value = engine.evaluate(f);
        if (value.isError())
            qWarning() << "Error in" << f << value.toString();
    }
    QString js = sigFuncName + "('" + s + "');";
    QJSValue value = engine.evaluate(js);
    bool error = false;
    if (value.isUndefined()) {
        qWarning() << "Undefined result for" << js;
        error = true;
    }
    if (value.isError()) {
        qWarning() << "Error in" << js << value.toString();
        error = true;
    }
    if (error) {
        QJSEngine engine2;
        engine2.evaluate(jsPlayer);
        value = engine2.evaluate(js);
        if (value.isUndefined()) {
            qWarning() << "Undefined result for" << js;
            error = true;
        }
        if (value.isError()) {
            qWarning() << "Error in" << js << value.toString();
            error = true;
        }
    }
    // jsPlayer.clear();
    if (error) return QString();
    return value.toString();
}

QString Video::formattedDuration() const {
    return DataUtils::formatDuration(m_duration);
}

void Video::saveDefinitionForUrl(const QString& url, const VideoDefinition& definition) {
    const QUrl videoUrl = QUrl::fromEncoded(url.toUtf8(), QUrl::StrictMode);
    m_streamUrl = videoUrl;
    definitionCode = definition.getCode();
    emit gotStreamUrl(videoUrl);
    loadingStreamUrl = false;
}

