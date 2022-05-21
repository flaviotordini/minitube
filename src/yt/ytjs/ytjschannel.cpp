#include "ytjschannel.h"

#include "js.h"

YTJSChannel::YTJSChannel(const QString &id, QObject *parent) : QObject(parent) {
    load(id);
}

void YTJSChannel::load(const QString &channelId) {
    JS::instance()
            .callFunction(new JSResult(this), "channelInfo", {channelId})
            .onJson([this](auto &doc) {
                auto obj = doc.object();

                displayName = obj["author"].toString();
                description = obj["description"].toString();

                const auto thumbs = obj["authorThumbnails"].toArray();
                int maxFoundWidth = 0;
                for (const auto &v : thumbs) {
                    auto thumbObj = v.toObject();
                    int width = thumbObj["width"].toInt();
                    if (width > maxFoundWidth) {
                        maxFoundWidth = width;
                        QString url = thumbObj["url"].toString();
                        thumbnailUrl = url;
                    }
                }

                emit loaded();
            })
            .onError([this](auto &msg) { emit error(msg); });
}
