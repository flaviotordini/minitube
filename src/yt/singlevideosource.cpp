#include "singlevideosource.h"

#include "video.h"
#include "videoapi.h"

#include "ivsinglevideosource.h"
#include "ytjssinglevideosource.h"
#include "ytsinglevideosource.h"

SingleVideoSource::SingleVideoSource(QObject *parent) : VideoSource(parent) {}

void SingleVideoSource::setVideo(Video *value) {
    emittedVideoIds.clear();
    video = value;
    // some wrapped source could delete the video
    connect(video, &QObject::destroyed, this, [this] {
        video = nullptr;
        videoId.clear();
    });
    videoId = video->getId();
}

void SingleVideoSource::setVideoId(const QString &value) {
    emittedVideoIds.clear();
    videoId = value;
}

void SingleVideoSource::loadVideos(int max, int startIndex) {
    if (!source) {
        aborted = false;
        if (VideoAPI::impl() == VideoAPI::YT3) {
            auto s = setupSource(new YTSingleVideoSource());
            s->setAsyncDetails(true);
            source = s;
        } else if (VideoAPI::impl() == VideoAPI::IV) {
            source = setupSource(new IVSingleVideoSource());
        } else if (VideoAPI::impl() == VideoAPI::JS) {
            source = setupSource(new YTJSSingleVideoSource());
        }
        connectSource(max, startIndex);
    }
    source->loadVideos(max, startIndex);
}

bool SingleVideoSource::hasMoreVideos() {
    if (source) return source->hasMoreVideos();
    return VideoSource::hasMoreVideos();
}

QString SingleVideoSource::getName() {
    if (source) return source->getName();
    return QString();
}

const QList<QAction *> &SingleVideoSource::getActions() {
    if (source) return source->getActions();
    return VideoSource::getActions();
}

int SingleVideoSource::maxResults() {
    if (source) return source->maxResults();
    return VideoSource::maxResults();
}

void SingleVideoSource::connectSource(int max, int startIndex) {
    connect(source, &VideoSource::finished, this, &VideoSource::finished);
    connect(source, &VideoSource::gotVideos, this, [this](auto &videos) {
        if (aborted) return;
        // avoid emitting duplicate videos
        auto videosCopy = videos;
        for (auto v : videos) {
            if (emittedVideoIds.contains(v->getId())) {
                videosCopy.removeOne(v);
            } else {
                emittedVideoIds << v->getId();
            }
        }
        emit gotVideos(videosCopy);
    });
    connect(source, &VideoSource::error, this, [this, max, startIndex](auto msg) {
        qDebug() << source << msg;
        if (aborted) return;
        if (QLatin1String(source->metaObject()->className()).startsWith(QLatin1String("YTJS"))) {
            qDebug() << "Falling back to IV";
            source->deleteLater();

            source = setupSource(new IVSingleVideoSource());

            connectSource(max, startIndex);
            source->loadVideos(max, startIndex);
        } else {
            emit error(msg);
        }
    });
}
