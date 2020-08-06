#include "ivsinglevideosource.h"

#include "http.h"
#include "httputils.h"
#include "video.h"

#include "invidious.h"
#include "ivlistparser.h"

IVSingleVideoSource::IVSingleVideoSource(QObject *parent)
    : VideoSource(parent), video(nullptr), startIndex(0), max(0) {}

void IVSingleVideoSource::loadVideos(int max, int startIndex) {
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
        url.setPath(url.path() + videoId);

    } else {
        url = Invidious::instance().method("videos");
        url.setPath(url.path() + "/" + videoId);
    }

    QObject *reply = Invidious::cachedHttp().get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(parseResults(QByteArray)));
    connect(reply, SIGNAL(error(QString)), SLOT(requestError(QString)));
}

void IVSingleVideoSource::parseResults(QByteArray data) {
    if (aborted) return;

    QJsonDocument doc = QJsonDocument::fromJson(data);
    const QJsonArray items = doc.object()["recommendedVideos"].toArray();
    IVListParser parser(items);
    const QVector<Video *> &videos = parser.getVideos();

    emit gotVideos(videos);
    if (startIndex == 1)
        loadVideos(max - 1, 2);
    else if (startIndex == 2)
        emit finished(videos.size() + 1);
    else
        emit finished(videos.size());
}

void IVSingleVideoSource::abort() {
    aborted = true;
}

QString IVSingleVideoSource::getName() {
    return name;
}

void IVSingleVideoSource::setVideo(Video *video) {
    this->video = video;
    videoId = video->getId();
}

void IVSingleVideoSource::requestError(const QString &message) {
    emit error(message);
}
