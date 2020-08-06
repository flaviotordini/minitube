#include "ivlistparser.h"

#include "video.h"

namespace {

QString decodeEntities(const QString &s) {
    return QTextDocumentFragment::fromHtml(s).toPlainText();
}

} // namespace

IVListParser::IVListParser(const QJsonArray &items) {
    videos.reserve(items.size());
    for (const QJsonValue &v : items) {
        QJsonObject item = v.toObject();
        parseItem(item);
    }
}

void IVListParser::parseItem(const QJsonObject &item) {
    Video *video = new Video();

    QJsonValue id = item[QLatin1String("videoId")];
    video->setId(id.toString());

    bool isLiveBroadcastContent = item[QLatin1String("liveNow")].toBool();
    if (isLiveBroadcastContent) {
        delete video;
        return;
    }

    int publishedAt = item[QLatin1String("published")].toInt();
    QDateTime publishedDateTime = QDateTime::fromSecsSinceEpoch(publishedAt);
    video->setPublished(publishedDateTime);

    video->setChannelId(item[QLatin1String("authorId")].toString());

    QString title = item[QLatin1String("title")].toString();
    static const QChar ampersand('&');
    if (title.contains(ampersand)) title = decodeEntities(title);
    video->setTitle(title);
    video->setDescription(item[QLatin1String("descriptionHtml")].toString());

    const auto thumbnails = item[QLatin1String("videoThumbnails")].toArray();
    for (const auto &thumbnail : thumbnails) {
        auto q = thumbnail["quality"];
        if (q == QLatin1String("medium")) {
            video->setThumbnailUrl(thumbnail["url"].toString());
        } else if (q == QLatin1String("high")) {
            video->setMediumThumbnailUrl(thumbnail["url"].toString());
        } else if (q == QLatin1String("sddefault")) {
            video->setLargeThumbnailUrl(thumbnail["url"].toString());
        }
    }

    video->setChannelTitle(item[QLatin1String("author")].toString());

    // These are only for "videos" requests

    int duration = item[QLatin1String("lengthSeconds")].toInt();
    video->setDuration(duration);

    int viewCount = item[QLatin1String("viewCount")].toInt();
    video->setViewCount(viewCount);

    videos.append(video);
}
