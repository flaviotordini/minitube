#include "ivvideolist.h"

#include "http.h"
#include "httputils.h"
#include "invidious.h"
#include "ivlistparser.h"
#include "video.h"

IVVideoList::IVVideoList(const QString &req, const QString &name, QObject *parent)
    : IVVideoSource(parent), name(name), req(req) {}

void IVVideoList::reallyLoadVideos(int max, int startIndex) {
    aborted = false;

    QUrl url(Invidious::instance().baseUrl() + req);

    auto *reply = Invidious::cachedHttp().get(url);
    connect(reply, &HttpReply::data, this, [this](auto data) {
        QJsonDocument doc = QJsonDocument::fromJson(data);
        const QJsonArray items = doc.array();
        if (items.isEmpty()) {
            handleError("No videos");
            return;
        }

        IVListParser parser(items);
        const QVector<Video *> &videos = parser.getVideos();

        emit gotVideos(videos);
        emit finished(videos.size());
    });
    connect(reply, &HttpReply::error, this, &IVVideoList::handleError);
}
