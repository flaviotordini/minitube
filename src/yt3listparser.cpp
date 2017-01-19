#include "yt3listparser.h"
#include "video.h"
#include "datautils.h"

YT3ListParser::YT3ListParser(const QByteArray &bytes) {
    QJsonDocument doc = QJsonDocument::fromJson(bytes);
    QJsonObject obj = doc.object();

    nextPageToken = obj["nextPageToken"].toString();

    QJsonArray items = obj["items"].toArray();
    videos.reserve(items.size());
    foreach (const QJsonValue &v, items) {
        QJsonObject item = v.toObject();
        parseItem(item);
    }

    // TODO suggestions!
}

void YT3ListParser::parseItem(const QJsonObject &item) {
    Video *video = new Video();

    QJsonValue id = item["id"];
    if (id.isString()) video->setId(id.toString());
    else {
        QString videoId = id.toObject()["videoId"].toString();
        video->setId(videoId);
    }

    QJsonObject snippet = item["snippet"].toObject();

    bool isLiveBroadcastContent = snippet["liveBroadcastContent"].toString() != QLatin1String("none");
    if (isLiveBroadcastContent) {
        delete video;
        return;
    }

    QString publishedAt = snippet["publishedAt"].toString();
    QDateTime publishedDateTime = QDateTime::fromString(publishedAt, Qt::ISODate);
    video->setPublished(publishedDateTime);

    video->setChannelId(snippet["channelId"].toString());

    video->setTitle(snippet["title"].toString());
    video->setDescription(snippet["description"].toString());

    QJsonObject thumbnails = snippet["thumbnails"].toObject();
    video->setThumbnailUrl(thumbnails["medium"].toObject()["url"].toString());
    video->setMediumThumbnailUrl(thumbnails["high"].toObject()["url"].toString());
    video->setLargeThumbnailUrl(thumbnails["standard"].toObject()["url"].toString());

    video->setChannelTitle(snippet["channelTitle"].toString());

    // These are only for "videos" requests

    QJsonValue contentDetails = item["contentDetails"];
    if (contentDetails.isObject()) {
        QString isoPeriod = contentDetails.toObject()["duration"].toString();
        int duration = DataUtils::parseIsoPeriod(isoPeriod);
        video->setDuration(duration);
    }

    QJsonValue statistics = item["statistics"];
    if (statistics.isObject()) {
        int viewCount = statistics.toObject()["viewCount"].toString().toInt();
        video->setViewCount(viewCount);
    }

    videos.append(video);
}
