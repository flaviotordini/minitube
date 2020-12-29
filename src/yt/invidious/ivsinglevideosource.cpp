#include "ivsinglevideosource.h"

#include "http.h"
#include "httputils.h"
#include "video.h"

#include "invidious.h"
#include "ivlistparser.h"

IVSingleVideoSource::IVSingleVideoSource(QObject *parent)
    : IVVideoSource(parent), video(nullptr), startIndex(0), max(0) {}

void IVSingleVideoSource::reallyLoadVideos(int max, int startIndex) {
    aborted = false;
    this->startIndex = startIndex;
    this->max = max;

    QUrl url;

    if (startIndex == 1) {
        if (video) {
            QVector<Video *> videos;
            videos << video->clone();
            if (name.isEmpty()) {
                name = videos.at(0)->getTitle();
                qDebug() << "Emitting name changed" << name;
                emit nameChanged(name);
            }
            emit gotVideos(videos);
            loadVideos(max - 1, 2);
            return;
        }

        url = Invidious::instance().method("videos/");
        if (url.isEmpty()) {
            QTimer::singleShot(500, this, [this] { handleError("No baseUrl"); });
            return;
        }
        url.setPath(url.path() + videoId);

    } else {
        url = Invidious::instance().method("videos");
        if (url.isEmpty()) {
            QTimer::singleShot(500, this, [this] { handleError("No baseUrl"); });
            return;
        }
        url.setPath(url.path() + "/" + videoId);
    }

    auto reply = Invidious::cachedHttp().get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(parseResults(QByteArray)));
    connect(reply, &HttpReply::error, this, &IVSingleVideoSource::handleError);
}

void IVSingleVideoSource::parseResults(QByteArray data) {
    if (aborted) return;

    QJsonDocument doc = QJsonDocument::fromJson(data);
    const QJsonArray items = doc.object()["recommendedVideos"].toArray();

    IVListParser parser(items);
    auto videos = parser.getVideos();

    emit gotVideos(videos);
    if (startIndex == 1)
        loadVideos(max - 1, 2);
    else if (startIndex == 2)
        emit finished(videos.size() + 1);
    else
        emit finished(videos.size());

    // fake more videos by loading videos related to the last one
    video->deleteLater();
    video = nullptr;
    if (!videos.isEmpty()) videoId = videos.last()->getId();
}

QString IVSingleVideoSource::getName() {
    return name;
}

void IVSingleVideoSource::setVideo(Video *video) {
    this->video = video;
    videoId = video->getId();
}
