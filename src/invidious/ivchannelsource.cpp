#include "ivchannelsource.h"

#include "http.h"
#include "httputils.h"
#include "invidious.h"
#include "ivlistparser.h"
#include "mainwindow.h"
#include "searchparams.h"
#include "video.h"

namespace {
int invidiousFixedMax = 20;
}

IVChannelSource::IVChannelSource(SearchParams *searchParams, QObject *parent)
    : IVVideoSource(parent), searchParams(searchParams) {
    searchParams->setParent(this);
}

void IVChannelSource::reallyLoadVideos(int max, int startIndex) {
    QUrl url = Invidious::instance().method("channels/videos/");
    url.setPath(url.path() + searchParams->channelId());

    QUrlQuery q(url);

    int page = ((startIndex - 1) / invidiousFixedMax) + 1;
    q.addQueryItem("page", QString::number(page));

    switch (searchParams->sortBy()) {
    case SearchParams::SortByNewest:
        q.addQueryItem("sort_by", "newest");
        break;
    case SearchParams::SortByViewCount:
        q.addQueryItem("sort_by", "popular");
        break;
    }

    url.setQuery(q);

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

        if (items.size() > invidiousFixedMax) invidiousFixedMax = items.size();

        if (name.isEmpty() && !searchParams->channelId().isEmpty()) {
            if (!videos.isEmpty()) {
                name = videos.at(0)->getChannelTitle();
                if (!searchParams->keywords().isEmpty()) {
                    name += QLatin1String(": ") + searchParams->keywords();
                }
            }
            emit nameChanged(name);
        }

        emit gotVideos(videos);
        emit finished(videos.size());
    });
    connect(reply, &HttpReply::error, this, &IVChannelSource::handleError);
}

QString IVChannelSource::getName() {
    return name;
}

const QList<QAction *> &IVChannelSource::getActions() {
    static const QList<QAction *> channelActions = {
            MainWindow::instance()->getAction("subscribeChannel")};
    if (searchParams->channelId().isEmpty()) {
        static const QList<QAction *> noActions;
        return noActions;
    }
    return channelActions;
}

int IVChannelSource::maxResults() {
    return invidiousFixedMax;
}
