#include "ytjstrending.h"

#include "js.h"
#include "video.h"

namespace {

QDateTime parsePublishedText(const QString &s) {
    int num = 0;
    const auto parts = s.split(' ');
    for (const auto &part : parts) {
        num = part.toInt();
        if (num > 0) break;
    }
    if (num == 0) return QDateTime();

    auto now = QDateTime::currentDateTimeUtc();
    if (s.contains("hour")) {
        return now.addSecs(-num * 3600);
    } else if (s.contains("day")) {
        return now.addDays(-num);
    } else if (s.contains("week")) {
        return now.addDays(-num * 7);
    } else if (s.contains("month")) {
        return now.addMonths(-num);
    } else if (s.contains("year")) {
        return now.addDays(-num * 365);
    }
    return QDateTime();
}

} // namespace

YTJSTrending::YTJSTrending(QString name, QVariantMap params, QObject *parent)
    : VideoSource(parent), name(name), params(params) {}

void YTJSTrending::loadVideos(int max, int startIndex) {
    aborted = false;

    auto &js = JS::instance();

    QJSValue options = js.getEngine().toScriptValue(params);

    js.callFunction(new JSResult(this), "trending", {options})
            .onJson([this](auto &doc) {
                const auto items = doc.array();

                QVector<Video *> videos;
                videos.reserve(items.size());

                for (const auto &v : items) {
                    auto i = v.toObject();

                    QString type = i["type"].toString();
                    if (type != "video") continue;

                    Video *video = new Video();

                    QString id = i["videoId"].toString();
                    video->setId(id);

                    QString title = i["title"].toString();
                    video->setTitle(title);

                    QString desc = i["description"].toString();
                    if (desc.isEmpty()) desc = i["desc"].toString();
                    video->setDescription(desc);

                    const auto thumbs = i["videoThumbnails"].toArray();
                    for (const auto &v : thumbs) {
                        auto t = v.toObject();
                        video->addThumb(t["width"].toInt(), t["height"].toInt(),
                                        t["url"].toString());
                    }

                    int views = i["viewCount"].toInt();
                    video->setViewCount(views);

                    int duration = i["lengthSeconds"].toInt();
                    video->setDuration(duration);

                    auto published = parsePublishedText(i["publishedText"].toString());
                    if (published.isValid()) video->setPublished(published);

                    QString channelName = i["author"].toString();
                    video->setChannelTitle(channelName);
                    QString channelId = i["authorId"].toString();
                    video->setChannelId(channelId);

                    videos << video;
                }

                emit gotVideos(videos);
                emit finished(videos.size());
            })
            .onError([this, max, startIndex](auto &msg) {
                static int retries = 0;
                if (retries < 3) {
                    qDebug() << "Retrying...";
                    QTimer::singleShot(0, this,
                                       [this, max, startIndex] { loadVideos(max, startIndex); });
                    retries++;
                } else {
                    emit error(msg);
                }
            });
}
