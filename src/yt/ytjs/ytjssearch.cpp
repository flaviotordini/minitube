#include "ytjssearch.h"

#include "mainwindow.h"
#include "searchparams.h"
#include "video.h"
#include "ytsearch.h"

#include "js.h"

namespace {

int parseDuration(const QString &s) {
    const auto parts = s.splitRef(':');
    int secs = 0;
    int p = 0;
    for (auto i = parts.crbegin(); i != parts.crend(); ++i) {
        if (p == 0) {
            secs = i->toInt();
        } else if (p == 1) {
            secs += i->toInt() * 60;
        } else if (p == 2) {
            secs += i->toInt() * 60 * 60;
        }
        p++;
    }
    return secs;
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

YTJSSearch::YTJSSearch(SearchParams *searchParams, QObject *parent)
    : VideoSource(parent), searchParams(searchParams) {}

void YTJSSearch::loadVideos(int max, int startIndex) {
    aborted = false;

    auto &js = JS::instance();
    auto &engine = js.getEngine();

    QString q;
    if (!searchParams->keywords().isEmpty()) {
        if (searchParams->keywords().startsWith("http://") ||
            searchParams->keywords().startsWith("https://")) {
            q = YTSearch::videoIdFromUrl(searchParams->keywords());
        } else
            q = searchParams->keywords();
    }

    // Options

    QJSValue options = engine.newObject();

    if (startIndex > 1) {
        if (!nextpageRef.isEmpty())
            options.setProperty("nextpageRef", nextpageRef);
        else {
            // non-first page was requested but we have no continuation token
            emit error("No pagination token");
            return;
        }
    }
    options.setProperty("limit", max);

    switch (searchParams->safeSearch()) {
    case SearchParams::None:
        options.setProperty("safeSearch", false);
        break;
    case SearchParams::Strict:
        options.setProperty("safeSearch", true);
        break;
    }

    // Filters

    auto filterMap = engine.evaluate("new Map()");
    auto jsMapSet = filterMap.property("set");
    auto addFilter = [&filterMap, &jsMapSet](QString name, QString value) {
        jsMapSet.callWithInstance(filterMap, {name, value});
    };

    addFilter("Type", "Video");

    switch (searchParams->sortBy()) {
    case SearchParams::SortByNewest:
        addFilter("Sort by", "Upload date");
        break;
    case SearchParams::SortByViewCount:
        addFilter("Sort by", "View count");
        break;
    case SearchParams::SortByRating:
        addFilter("Sort by", "Rating");
        break;
    }

    switch (searchParams->duration()) {
    case SearchParams::DurationShort:
        addFilter("Duration", "Short");
        break;
    case SearchParams::DurationMedium:
    case SearchParams::DurationLong:
        addFilter("Duration", "Long");
        break;
    }

    switch (searchParams->time()) {
    case SearchParams::TimeToday:
        addFilter("Upload date", "Today");
        break;
    case SearchParams::TimeWeek:
        addFilter("Upload date", "This week");
        break;
    case SearchParams::TimeMonth:
        addFilter("Upload date", "This month");
        break;
    }

    switch (searchParams->quality()) {
    case SearchParams::QualityHD:
        addFilter("Features", "HD");
        break;
    case SearchParams::Quality4K:
        addFilter("Features", "4K");
        break;
    case SearchParams::QualityHDR:
        addFilter("Features", "HDR");
        break;
    }

    js.callFunction(new JSResult(this), "search", {q, options, filterMap})
            .onJson([this](auto &doc) {
                if (aborted) return;

                auto obj = doc.object();

                nextpageRef = obj["nextpageRef"].toString();

                const auto items = obj["items"].toArray();
                QVector<Video *> videos;
                videos.reserve(items.size());

                for (const auto &i : items) {
                    QString type = i["type"].toString();
                    if (type != "video") continue;

                    Video *video = new Video();

                    QString id = YTSearch::videoIdFromUrl(i["link"].toString());
                    video->setId(id);

                    QString title = i["title"].toString();
                    video->setTitle(title);

                    QString desc = i["description"].toString();
                    video->setDescription(desc);

                    QString thumb = i["thumbnail"].toString();
                    video->setThumbnailUrl(thumb);

                    int views = i["views"].toInt();
                    video->setViewCount(views);

                    int duration = parseDuration(i["duration"].toString());
                    video->setDuration(duration);

                    auto published = parsePublishedText(i["uploaded_at"].toString());
                    if (published.isValid()) video->setPublished(published);

                    auto authorObj = i["author"];
                    QString channelName = authorObj["name"].toString();
                    video->setChannelTitle(channelName);
                    QString channelId = parseChannelId(authorObj["ref"].toString());
                    video->setChannelId(channelId);

                    videos << video;
                }

                if (videos.isEmpty()) {
                    emit error("No results");
                } else {
                    emit gotVideos(videos);
                    emit finished(videos.size());
                }
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

QString YTJSSearch::getName() {
    if (!name.isEmpty()) return name;
    if (!searchParams->keywords().isEmpty()) return searchParams->keywords();
    return QString();
}

const QList<QAction *> &YTJSSearch::getActions() {
    static const QList<QAction *> channelActions = {
            MainWindow::instance()->getAction("subscribeChannel")};
    if (searchParams->channelId().isEmpty()) {
        static const QList<QAction *> noActions;
        return noActions;
    }
    return channelActions;
}
