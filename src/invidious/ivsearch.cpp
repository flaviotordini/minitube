#include "ivsearch.h"

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

IVSearch::IVSearch(SearchParams *searchParams, QObject *parent)
    : VideoSource(parent), searchParams(searchParams) {
    searchParams->setParent(this);
}

void IVSearch::loadVideos(int max, int startIndex) {
    aborted = false;

    QUrl url = Invidious::instance().method("search");

    QUrlQuery q(url);

    // invidious always returns 20 results
    int page = ((startIndex - 1) / invidiousFixedMax) + 1;
    q.addQueryItem("page", QString::number(page));

    if (!searchParams->keywords().isEmpty()) {
        q.addQueryItem("q", searchParams->keywords());
    }

    if (!searchParams->channelId().isEmpty())
        q.addQueryItem("channelId", searchParams->channelId());

    switch (searchParams->sortBy()) {
    case SearchParams::SortByNewest:
        q.addQueryItem("sort_by", "upload_date");
        break;
    case SearchParams::SortByViewCount:
        q.addQueryItem("sort_by", "view_count");
        break;
    case SearchParams::SortByRating:
        q.addQueryItem("sort_by", "rating");
        break;
    }

    switch (searchParams->duration()) {
    case SearchParams::DurationShort:
        q.addQueryItem("duration", "short");
        break;
    case SearchParams::DurationMedium:
        q.addQueryItem("duration", "medium");
        break;
    case SearchParams::DurationLong:
        q.addQueryItem("duration", "long");
        break;
    }

    switch (searchParams->time()) {
    case SearchParams::TimeToday:
        q.addQueryItem("date", "today");
        break;
    case SearchParams::TimeWeek:
        q.addQueryItem("date", "week");
        break;
    case SearchParams::TimeMonth:
        q.addQueryItem("date", "month");
        break;
    }

    switch (searchParams->quality()) {
    case SearchParams::QualityHD:
        q.addQueryItem("features", "hd");
        break;
    }

    url.setQuery(q);

    // qWarning() << "YT3 search" << url.toString();
    QObject *reply = Invidious::cachedHttp().get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(parseResults(QByteArray)));
    connect(reply, SIGNAL(error(QString)), SLOT(requestError(QString)));
}

void IVSearch::parseResults(const QByteArray &data) {
    if (aborted) return;

    QJsonDocument doc = QJsonDocument::fromJson(data);
    const QJsonArray items = doc.array();
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
}

void IVSearch::abort() {
    aborted = true;
}

QString IVSearch::getName() {
    if (!name.isEmpty()) return name;
    if (!searchParams->keywords().isEmpty()) return searchParams->keywords();
    return QString();
}

void IVSearch::requestError(const QString &message) {
    Invidious::instance().initServers();
    QString msg = message;
    emit error(msg);
}

const QList<QAction *> &IVSearch::getActions() {
    static const QList<QAction *> channelActions = {
            MainWindow::instance()->getAction("subscribeChannel")};
    if (searchParams->channelId().isEmpty()) {
        static const QList<QAction *> noActions;
        return noActions;
    }
    return channelActions;
}

int IVSearch::maxResults() {
    return invidiousFixedMax;
}
