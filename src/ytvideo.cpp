#include "ytvideo.h"

#include "datautils.h"
#include "http.h"
#include "httputils.h"
#include "jsfunctions.h"
#include "temporary.h"
#include "videodefinition.h"
#include "yt3.h"

#include <QJSEngine>
#include <QJSValue>
#include <QtNetwork>

namespace {
static const QString jsNameChars = "a-zA-Z0-9\\$_";
}

YTVideo::YTVideo(const QString &videoId, QObject *parent)
    : QObject(parent), videoId(videoId), definitionCode(0), elIndex(0), ageGate(false),
      loadingStreamUrl(false) {}

void YTVideo::loadStreamUrl() {
    if (loadingStreamUrl) {
        qDebug() << "Already loading stream URL for" << videoId;
        return;
    }
    loadingStreamUrl = true;
    elIndex = 0;
    ageGate = false;

    getVideoInfo();
}

void YTVideo::getVideoInfo() {
    static const QStringList elTypes = {"&el=embedded", "&el=detailpage", "&el=vevo", ""};

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
        url = QUrl(QString("https://www.youtube.com/"
                           "get_video_info?video_id=%1%2&ps=default&eurl=&gl=US&hl=en")
                           .arg(videoId, elTypes.at(elIndex)));
    }

    QObject *reply = HttpUtils::yt().get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(gotVideoInfo(QByteArray)));
    connect(reply, SIGNAL(error(QString)), SLOT(errorVideoInfo(QString)));

    // see you in gotVideoInfo...
}

void YTVideo::gotVideoInfo(const QByteArray &bytes) {
    QString videoInfo = QString::fromUtf8(bytes);
    // qDebug() << "videoInfo" << videoInfo;

    // get video token
    static const QRegExp videoTokeRE(JsFunctions::instance()->videoTokenRE());
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
    static const QRegExp fmtMapRE(JsFunctions::instance()->videoInfoFmtMapRE());
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

void YTVideo::parseFmtUrlMap(const QString &fmtUrlMap, bool fromWebPage) {
    int videoFormat = 0;
    const VideoDefinition &definition = YT3::instance().maxVideoDefinition();

    // qDebug() << "fmtUrlMap" << fmtUrlMap;
    const QVector<QStringRef> formatUrls = fmtUrlMap.splitRef(',', QString::SkipEmptyParts);

    for (const QStringRef &formatUrl : formatUrls) {
        // qDebug() << "formatUrl" << formatUrl;
        const QVector<QStringRef> urlParams = formatUrl.split('&', QString::SkipEmptyParts);
        // qDebug() << "urlParams" << urlParams;

        int format = -1;
        QString url;
        QString sig;
        for (const QStringRef &urlParam : urlParams) {
            // qWarning() << "urlParam" << urlParam;
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
                        if (sig.isEmpty()) sig = JsFunctions::instance()->decryptSignature(sig);
                    }
                } else {
                    loadWebPage();
                    return;
                }
            }
        }
        if (format == -1 || url.isNull()) continue;

        url += QLatin1String("&signature=") + sig;

        if (!url.contains(QLatin1String("ratebypass"))) url += QLatin1String("&ratebypass=yes");

        qDebug() << format;
        if (format == definition.getCode()) {
            qDebug() << "Found format" << format;
            if (definition.getAudioCode() == 0) {
                // we found the exact match with an audio/video stream
                saveDefinitionForUrl(url, definition);
                return;
            }
            videoFormat = format;
        }
        urlMap.insert(format, url);
    }

    if (!fromWebPage) {
        loadWebPage();
        return;
    }

    if (videoFormat != 0) {
        // exact match with video stream was found
        const VideoDefinition &definition = VideoDefinition::forCode(videoFormat);
        saveDefinitionForUrl(urlMap.value(videoFormat), definition);
        return;
    }

    const QVector<VideoDefinition> &definitions = VideoDefinition::getDefinitions();
    int previousIndex = std::max(definitions.indexOf(definition) - 1, 0);
    for (; previousIndex >= 0; previousIndex--) {
        const VideoDefinition &previousDefinition = definitions.at(previousIndex);
        qDebug() << "Testing format" << previousDefinition.getCode();
        if (urlMap.contains(previousDefinition.getCode())) {
            qDebug() << "Found format" << previousDefinition.getCode();
            saveDefinitionForUrl(urlMap.value(previousDefinition.getCode()), previousDefinition);
            return;
        }
    }

    emit errorStreamUrl(tr("Cannot get video stream for %1").arg(videoId));
}

void YTVideo::loadWebPage() {
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
}

void YTVideo::errorVideoInfo(const QString &message) {
    loadingStreamUrl = false;
    emit errorStreamUrl(message);
}

void YTVideo::scrapeWebPage(const QByteArray &bytes) {
    qDebug() << "scrapeWebPage";

    const QString html = QString::fromUtf8(bytes);

    static const QRegExp ageGateRE(JsFunctions::instance()->ageGateRE());
    if (ageGateRE.indexIn(html) != -1) {
        qDebug() << "Found ageGate";
        ageGate = true;
        elIndex = 4;
        getVideoInfo();
        return;
    }

    // "\"url_encoded_fmt_stream_map\":\s*\"([^\"]+)\""
    static const QRegExp fmtMapRE(JsFunctions::instance()->webPageFmtMapRE());
    if (fmtMapRE.indexIn(html) != -1) {
        fmtUrlMap = fmtMapRE.cap(1);
        fmtUrlMap.replace("\\u0026", "&");
    }

    QRegExp adaptiveFormatsRE("\"adaptive_fmts\":\\s*\"([^\"]+)\"");
    if (adaptiveFormatsRE.indexIn(html) != -1) {
        qDebug() << "Found adaptive_fmts";
        if (!fmtUrlMap.isEmpty()) fmtUrlMap += ',';
        fmtUrlMap += adaptiveFormatsRE.cap(1).replace("\\u0026", "&");
    }

    if (fmtUrlMap.isEmpty()) {
        qWarning() << "Error parsing video page";
        // emit errorStreamUrl("Error parsing video page");
        // loadingStreamUrl = false;
        elIndex++;
        getVideoInfo();
        return;
    }

    static const QRegExp jsPlayerRe(JsFunctions::instance()->jsPlayerRE());
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

void YTVideo::parseJsPlayer(const QByteArray &bytes) {
    jsPlayer = QString::fromUtf8(bytes);
    // qDebug() << "jsPlayer" << jsPlayer;

    // QRegExp funcNameRe("[\"']signature[\"']\\s*,\\s*([" + jsNameChars + "]+)\\(");
    static const QRegExp funcNameRe(
            JsFunctions::instance()->signatureFunctionNameRE().arg(jsNameChars));

    // qDebug() << "funcNameRe" << funcNameRe;

    if (funcNameRe.indexIn(jsPlayer) == -1) {
        qWarning() << "Cannot capture signature function name" << jsPlayer;
    } else {
        sigFuncName = funcNameRe.cap(1);

        // qDebug() << "Captures" << funcNameRe.captureCount() << funcNameRe.capturedTexts();

        if (sigFuncName.isEmpty()) {
            qWarning() << "Empty signature function name" << jsPlayer;
        }

        captureFunction(sigFuncName, jsPlayer);
        // qWarning() << sigFunctions << sigObjects;
    }

    parseFmtUrlMap(fmtUrlMap, true);
}

void YTVideo::captureFunction(const QString &name, const QString &js) {
    qDebug() << __PRETTY_FUNCTION__ << name;
    const QString argsAndBody =
            QLatin1String("\\s*\\([") + jsNameChars + QLatin1String(",\\s]*\\)\\s*\\{[^\\}]+\\}");
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
    static const QRegExp invokedFuncRe(QLatin1String("[\\s=;\\(]([") + jsNameChars +
                                       QLatin1String("]+)\\s*\\([") + jsNameChars +
                                       QLatin1String(",\\s]+\\)"));
    int pos = name.length() + 9;
    while ((pos = invokedFuncRe.indexIn(func, pos)) != -1) {
        QString funcName = invokedFuncRe.cap(1);
        if (!sigFunctions.contains(funcName)) captureFunction(funcName, js);
        pos += invokedFuncRe.matchedLength();
    }

    // capture referenced objects
    static const QRegExp objRe(QLatin1String("[\\s=;\\(]([") + jsNameChars +
                               QLatin1String("]+)\\.[") + jsNameChars + QLatin1String("]+"));
    pos = name.length() + 9;
    while ((pos = objRe.indexIn(func, pos)) != -1) {
        QString objName = objRe.cap(1);
        if (!sigObjects.contains(objName)) captureObject(objName, js);
        pos += objRe.matchedLength();
    }
}

void YTVideo::captureObject(const QString &name, const QString &js) {
    QRegExp re(QLatin1String("var\\s+") + QRegExp::escape(name) +
               QLatin1String("\\s*=\\s*\\{.*\\}\\s*;"));
    re.setMinimal(true);
    if (re.indexIn(js) == -1) {
        qWarning() << "Cannot capture object" << name;
        return;
    }
    QString obj = re.cap(0);
    sigObjects.insert(name, obj);
}

QString YTVideo::decryptSignature(const QString &s) {
    qDebug() << "decryptSignature" << sigFuncName << sigFunctions << sigObjects;
    if (sigFuncName.isEmpty()) return QString();
    QJSEngine engine;
    for (const QString &f : sigObjects) {
        QJSValue value = engine.evaluate(f);
        if (value.isError()) qWarning() << "Error in" << f << value.toString();
    }
    for (const QString &f : sigFunctions) {
        QJSValue value = engine.evaluate(f);
        if (value.isError()) qWarning() << "Error in" << f << value.toString();
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
    if (error) return QString();
    return value.toString();
}

void YTVideo::saveDefinitionForUrl(const QString &url, const VideoDefinition &definition) {
    qDebug() << "Selected video format" << definition.getCode() << definition.getName()
             << definition.getAudioCode();
    m_streamUrl = url;
    definitionCode = definition.getCode();

    QString audioUrl;
    if (definition.getAudioCode() != 0) {
        qDebug() << "Finding audio format";
        static const QVector<int> audioFormats({140, 171, 251});
        for (int audioFormat : audioFormats) {
            qDebug() << "Trying audio format" << audioFormat;
            auto i = urlMap.constFind(audioFormat);
            if (i != urlMap.constEnd()) {
                qDebug() << "Found audio format" << i.value();
                audioUrl = i.value();
                break;
            }
        }
    }

    loadingStreamUrl = false;
    emit gotStreamUrl(url, audioUrl);
}
