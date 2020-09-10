#include "ytjssinglevideosource.h"

#include "video.h"
#include "ytjs.h"

YTJSSingleVideoSource::YTJSSingleVideoSource(QObject *parent)
    : VideoSource(parent), video(nullptr) {}

void YTJSSingleVideoSource::loadVideos(int max, int startIndex) {
    aborted = false;

    auto &ytjs = YTJS::instance();
    if (!ytjs.isInitialized()) {
        QTimer::singleShot(500, this, [this, max, startIndex] { loadVideos(max, startIndex); });
        return;
    }
    auto &engine = ytjs.getEngine();

    auto function = engine.evaluate("videoInfo");
    if (!function.isCallable()) {
        qWarning() << function.toString() << " is not callable";
        emit error(function.toString());
        return;
    }

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

    auto handler = new ResultHandler;
    connect(handler, &ResultHandler::error, this, &VideoSource::error);
    connect(handler, &ResultHandler::data, this, [this](const QJsonDocument &doc) {
        if (aborted) return;

        auto obj = doc.object();

        const auto items = obj["related_videos"].toArray();
        QVector<Video *> videos;
        videos.reserve(items.size());

        for (const auto &i : items) {
            Video *video = new Video();

            QString id = i["id"].toString();
            video->setId(id);

            QString title = i["title"].toString();
            video->setTitle(title);

            QString desc = i["description"].toString();
            if (desc.isEmpty()) desc = i["desc"].toString();
            video->setDescription(desc);

            QString thumb = i["video_thumbnail"].toString();
            video->setThumbnailUrl(thumb);

            int views = i["view_count"].toInt();
            video->setViewCount(views);

            int duration = i["length_seconds"].toInt();
            video->setViewCount(duration);

            QString channelId = i["ucid"].toString();
            video->setChannelId(channelId);

            QString channelName = i["author"].toString();
            video->setChannelTitle(channelName);

            videos << video;
        }

        emit gotVideos(videos);
        emit finished(videos.size());
    });
    QJSValue h = engine.newQObject(handler);
    auto value = function.call({h, videoId});
    ytjs.checkError(value);
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
