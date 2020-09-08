#include "ytjschannel.h"

#include "http.h"
#include "httputils.h"
#include "ytjs.h"

YTJSChannel::YTJSChannel(const QString &id, QObject *parent) : QObject(parent) {
    load(id);
}

void YTJSChannel::load(const QString &channelId) {
    auto &ytjs = YTJS::instance();
    if (!ytjs.isInitialized()) {
        QTimer::singleShot(500, this, [this, channelId] { load(channelId); });
        return;
    }
    auto &engine = ytjs.getEngine();

    auto function = engine.evaluate("channelInfo");
    if (!function.isCallable()) {
        qWarning() << function.toString() << " is not callable";
        emit error(function.toString());
        return;
    }

    auto handler = new ResultHandler;
    connect(handler, &ResultHandler::error, this, &YTJSChannel::error);
    connect(handler, &ResultHandler::data, this, [this](const QJsonDocument &doc) {
        auto obj = doc.object();

        displayName = obj["author"].toString();
        description = obj["description"].toString();

        const auto thumbs = obj["authorThumbnails"].toArray();
        int maxFoundWidth = 0;
        for (const auto &thumbObj : thumbs) {
            QString url = thumbObj["url"].toString();
            int width = thumbObj["width"].toInt();
            if (width > maxFoundWidth) {
                maxFoundWidth = width;
                thumbnailUrl = url;
            }
        }

        emit loaded();
    });
    QJSValue h = engine.newQObject(handler);
    auto value = function.call({h, channelId});
    ytjs.checkError(value);
}
