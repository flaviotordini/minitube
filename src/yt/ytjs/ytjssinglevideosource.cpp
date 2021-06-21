#include "ytjssinglevideosource.h"

#include "js.h"
#include "video.h"

namespace {

QDateTime parsePublishedText(const QString &s) {
    int num = 0;
    const auto parts = s.splitRef(' ');
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

YTJSSingleVideoSource::YTJSSingleVideoSource(QObject *parent)
    : VideoSource(parent), video(nullptr) {}

void YTJSSingleVideoSource::loadVideos(int max, int startIndex) {
    aborted = false;

    if (startIndex == 1) {
        if (video) {
            if (name.isEmpty()) {
                name = video->getTitle();
                qDebug() << "Emitting name changed" << name;
                emit nameChanged(name);
            }
            emit gotVideos({video->clone()});
        }
    }

    JS::instance()
            .callFunction(new JSResult(this), "videoInfo", {videoId})
            .onJson([this](auto &doc) {
                if (aborted) return;

                auto obj = doc.object();

                auto parseVideoObject = [](QJsonObject i) {
                    Video *video = new Video();

                    QString id = i["id"].toString();
                    if (id.isEmpty()) id = i["videoId"].toString();
                    video->setId(id);

                    QString title = i["title"].toString();
                    video->setTitle(title);

                    QString desc = i["description"].toString();
                    if (desc.isEmpty()) desc = i["desc"].toString();
                    if (desc.isEmpty()) desc = i["shortDescription"].toString();

                    video->setDescription(desc);

                    const auto thumbs = i["thumbnails"].toArray();
                    for (const auto &t : thumbs) {
                        video->addThumb(t["width"].toInt(), t["height"].toInt(),
                                        t["url"].toString());
                    }

                    qDebug() << i["view_count"] << i["viewCount"];
                    int views = i["view_count"].toString().toInt();
                    if (views == 0) views = i["viewCount"].toString().toInt();
                    video->setViewCount(views);

                    int duration = i["length_seconds"].toInt();
                    video->setDuration(duration);

                    auto author = i["author"];
                    if (author.isObject()) {
                        auto authorObject = author.toObject();
                        video->setChannelId(authorObject["id"].toString());
                        video->setChannelTitle(authorObject["name"].toString());
                    } else if (author.isString()) {
                        video->setChannelId(i["ucid"].toString());
                        video->setChannelTitle(author.toString());
                    }

                    auto published = parsePublishedText(i["published"].toString());
                    if (published.isValid()) video->setPublished(published);

                    return video;
                };

                QVector<Video *> videos;

                if (!video) {
                    // first video
                    auto video = parseVideoObject(obj["videoDetails"].toObject());
                    videos << video;
                    name = video->getTitle();
                    qDebug() << "Emitting name changed" << name;
                    emit nameChanged(name);
                }

                const auto items = obj["related_videos"].toArray();
                videos.reserve(items.size());

                for (const auto &i : items) {
                    videos << parseVideoObject(i.toObject());
                }

                if (videos.isEmpty()) {
                    emit error("No results");
                } else {
                    emit gotVideos(videos);
                    emit finished(videos.size());
                }

                // fake more videos by loading videos related to the last one
                video = nullptr;
                if (!videos.isEmpty()) videoId = videos.last()->getId();
            })
            .onError([this](auto &msg) { emit error(msg); });
}

void YTJSSingleVideoSource::setVideo(Video *video) {
    this->video = video;
    videoId = video->getId();
}

void YTJSSingleVideoSource::abort() {
    aborted = true;
}

QString YTJSSingleVideoSource::getName() {
    return name;
}
