#include "yt3listparser.h"
#include "video.h"
#include "datautils.h"

YT3ListParser::YT3ListParser(const QByteArray &bytes) {

    QScriptEngine engine;
    QScriptValue json = engine.evaluate("(" + QString::fromUtf8(bytes) + ")");

    nextPageToken = json.property("nextPageToken").toString();

    QScriptValue items = json.property("items");
    videos.reserve(items.property("length").toInt32() - 1);
    if (items.isArray()) {
        QScriptValueIterator it(items);
        while (it.hasNext()) {
            it.next();
            QScriptValue item = it.value();
            // For some reason the array has an additional element containing its size.
            if (item.isObject()) parseItem(item);
        }
    }

    // TODO suggestions!
}

void YT3ListParser::parseItem(const QScriptValue &item) {
    Video *video = new Video();

    QScriptValue id = item.property("id");
    if (id.isString()) video->setId(id.toString());
    else {
        QString videoId = id.property("videoId").toString();
        video->setId(videoId);
    }

    QScriptValue snippet = item.property("snippet");

    bool isLiveBroadcastContent = snippet.property("liveBroadcastContent").toString() != QLatin1String("none");
    if (isLiveBroadcastContent) {
        delete video;
        return;
    }

    QString publishedAt = snippet.property("publishedAt").toString();
    QDateTime publishedDateTime = QDateTime::fromString(publishedAt, Qt::ISODate);
    video->setPublished(publishedDateTime);

    video->setChannelId(snippet.property("channelId").toString());

    video->setTitle(snippet.property("title").toString());
    video->setDescription(snippet.property("description").toString());

    QScriptValue thumbnails = snippet.property("thumbnails");
    video->setThumbnailUrl(thumbnails.property("medium").property("url").toString());
    video->setMediumThumbnailUrl(thumbnails.property("high").property("url").toString());

    video->setChannelTitle(snippet.property("channelTitle").toString());

    // These are only for "videos" requests

    QScriptValue contentDetails = item.property("contentDetails");
    if (contentDetails.isObject()) {
        QString isoPeriod = contentDetails.property("duration").toString();
        int duration = DataUtils::parseIsoPeriod(isoPeriod);
        video->setDuration(duration);
    }

    QScriptValue statistics = item.property("statistics");
    if (statistics.isObject()) {
        uint viewCount = statistics.property("viewCount").toUInt32();
        video->setViewCount(viewCount);
    }

    videos.append(video);
}
