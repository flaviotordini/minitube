#include "ivvideolist.h"

#include "http.h"
#include "httputils.h"
#include "invidious.h"
#include "ivlistparser.h"
#include "video.h"

IVVideoList::IVVideoList(const QString &req, const QString &name, QObject *parent)
    : VideoSource(parent), name(name), req(req) {}

void IVVideoList::loadVideos(int max, int startIndex) {
    aborted = false;

    QUrl url(Invidious::instance().baseUrl() + req);

    auto *reply = Invidious::cachedHttp().get(url);
    connect(reply, &HttpReply::data, this, [this](auto data) {
        QJsonDocument doc = QJsonDocument::fromJson(data);
        const QJsonArray items = doc.array();
        IVListParser parser(items);
        const QVector<Video *> &videos = parser.getVideos();
        qDebug() << "CAOCAO" << req << name << videos.size();

        emit gotVideos(videos);
        emit finished(videos.size());
    });
    connect(reply, &HttpReply::error, this, [this](auto message) {
        Invidious::instance().initServers();
        emit error(message);
    });
}

void IVVideoList::abort() {
    aborted = true;
}
