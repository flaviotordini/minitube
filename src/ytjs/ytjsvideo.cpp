#include "ytjsvideo.h"

#include "videodefinition.h"
#include "yt3.h"
#include "ytjs.h"

YTJSVideo::YTJSVideo(const QString &videoId, QObject *parent)
    : QObject(parent), videoId(videoId), definitionCode(0) {}

void YTJSVideo::loadStreamUrl() {
    if (loadingStreamUrl) return;
    loadingStreamUrl = true;

    auto &ytjs = YTJS::instance();
    if (!ytjs.isInitialized()) {
        QTimer::singleShot(500, this, [this] { loadStreamUrl(); });
        return;
    }
    auto &engine = ytjs.getEngine();

    auto function = engine.evaluate("videoInfo");
    if (!function.isCallable()) {
        qWarning() << function.toString() << " is not callable";
        loadingStreamUrl = false;
        emit errorStreamUrl(function.toString());
        return;
    }

    auto handler = new ResultHandler;
    connect(handler, &ResultHandler::error, this, &YTJSVideo::errorStreamUrl);
    connect(handler, &ResultHandler::data, this, [this](const QJsonDocument &doc) {
        auto obj = doc.object();

        QMap<int, QString> urlMap;
        const auto formats = obj["formats"].toArray();
        for (const auto &format : formats) {
            bool isDashMpd = format["isDashMPD"].toBool();
            if (isDashMpd) continue;
            int itag = format["itag"].toInt();
            QString url = format["url"].toString();
            // qDebug() << itag << url;
            urlMap.insert(itag, url);
        }
        if (urlMap.isEmpty()) {
            loadingStreamUrl = false;
            emit errorStreamUrl("No formats");
            return;
        }

        qDebug() << "available formats" << urlMap.keys();
        const VideoDefinition &definition = YT3::instance().maxVideoDefinition();
        const QVector<VideoDefinition> &definitions = VideoDefinition::getDefinitions();
        int previousIndex = std::max(definitions.indexOf(definition), 0);
        for (; previousIndex >= 0; previousIndex--) {
            const VideoDefinition &previousDefinition = definitions.at(previousIndex);
            qDebug() << "Testing format" << previousDefinition.getCode();
            if (urlMap.contains(previousDefinition.getCode())) {
                qDebug() << "Found format" << previousDefinition.getCode();

                QString url = urlMap.value(previousDefinition.getCode());
                definitionCode = previousDefinition.getCode();

                QString audioUrl;
                if (!previousDefinition.hasAudio()) {
                    qDebug() << "Finding audio format";
                    static const QVector<int> audioFormats({251, 171, 140});
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
                return;
            }
        }

        loadingStreamUrl = false;
        emit errorStreamUrl(tr("Cannot get video stream for %1").arg(videoId));
    });
    QJSValue h = engine.newQObject(handler);
    auto value = function.call({h, videoId});
    ytjs.checkError(value);
}
