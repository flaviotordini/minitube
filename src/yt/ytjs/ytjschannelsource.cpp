#include "ytjschannelsource.h"

#include "mainwindow.h"
#include "searchparams.h"
#include "video.h"
#include "ytsearch.h"

#include "js.h"

namespace {

int parseDuration(const QString &s) {
    static const QTime zeroTime(0, 0);
    QTime time = QTime::fromString(s, QStringLiteral("hh:mm:ss"));
    return zeroTime.secsTo(time);
}

QString parseChannelId(const QString &channelUrl) {
    int pos = channelUrl.lastIndexOf('/');
    if (pos >= 0) return channelUrl.mid(pos + 1);
    return QString();
}

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

YTJSChannelSource::YTJSChannelSource(SearchParams *searchParams, QObject *parent)
    : VideoSource(parent), searchParams(searchParams) {}

void YTJSChannelSource::loadVideos(int max, int startIndex) {
    aborted = false;

    QString channelId = searchParams->channelId();

    QString sortBy;
    switch (searchParams->sortBy()) {
    case SearchParams::SortByNewest:
        sortBy = "newest";
        break;
    case SearchParams::SortByViewCount:
        sortBy = "oldest";
        break;
    case SearchParams::SortByRating:
        sortBy = "popular";
        break;
    }

    if (startIndex <= 1) continuation.clear();

    auto &js = JS::instance();

    js.callFunction(new JSResult(this), "channelVideos", {channelId, sortBy, continuation})
            .onJson([this](auto &doc) {
                auto obj = doc.object();

                continuation = obj["continuation"].toString();

                const auto items = obj["items"].toArray();
                QVector<Video *> videos;
                videos.reserve(items.size());

                for (const auto &i : items) {
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
                    for (const auto &thumbObj : thumbs) {
                        QString url = thumbObj["url"].toString();
                        int width = thumbObj["width"].toInt();
                        if (width >= 336)
                            video->setLargeThumbnailUrl(url);
                        else if (width >= 246)
                            video->setMediumThumbnailUrl(url);
                        else if (width >= 168)
                            video->setThumbnailUrl(url);
                    }

                    int views = i["viewCount"].toInt();
                    video->setViewCount(views);

                    int duration = i["lengthSeconds"].toInt();
                    video->setDuration(duration);

                    auto published = parsePublishedText(i["publishedText"].toString());
                    if (published.isValid()) video->setPublished(published);

                    QString channelName = i["author"].toString();
                    if (channelName != name) {
                        name = channelName;
                        emit nameChanged(name);
                    }
                    video->setChannelTitle(channelName);
                    QString channelId = i["authorId"].toString();
                    video->setChannelId(channelId);

                    videos << video;
                }

                emit gotVideos(videos);
                emit finished(videos.size());
            })
            .onError([this, &js, max, startIndex](auto &msg) {
                static int retries = 0;
                if (retries < 3) {
                    qDebug() << "Retrying...";
                    auto nam = js.getEngine().networkAccessManager();
                    nam->clearAccessCache();
                    nam->setCookieJar(new QNetworkCookieJar());
                    QTimer::singleShot(0, this,
                                       [this, max, startIndex] { loadVideos(max, startIndex); });
                    retries++;
                } else {
                    emit error(msg);
                }
            });
}

QString YTJSChannelSource::getName() {
    return name;
}

const QList<QAction *> &YTJSChannelSource::getActions() {
    static const QList<QAction *> channelActions = {
            MainWindow::instance()->getAction("subscribeChannel")};
    if (searchParams->channelId().isEmpty()) {
        static const QList<QAction *> noActions;
        return noActions;
    }
    return channelActions;
}
