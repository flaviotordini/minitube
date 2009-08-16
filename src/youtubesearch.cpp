#include "youtubesearch.h"
#include "youtubestreamreader.h"
#include "Constants.h"
#include "networkaccess.h"

namespace The {
    NetworkAccess* http();
}

YouTubeSearch::YouTubeSearch() : QObject() {}

void YouTubeSearch::search(SearchParams *searchParams, int max, int skip) {
    this->abortFlag = false;

    QString urlString = QString(
            "http://gdata.youtube.com/feeds/api/videos?q=%1&max-results=%2&start-index=%3")
            .arg(searchParams->keywords(), QString::number(max), QString::number(skip));

    // Useful to test with a local webserver
    /*
    urlString = QString("http://localhost/oringo/video.xml?q=%1&max-results=%2&start-index=%3")
                .arg(searchParams->keywords(), QString::number(max), QString::number(skip));
                */

    switch (searchParams->sortBy()) {
    case SearchParams::SortByNewest:
        urlString.append("&orderby=published");
        break;
    case SearchParams::SortByViewCount:
        urlString.append("&orderby=viewCount");
        break;
    }

    QUrl url(urlString);

    QObject *reply = The::http()->get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(parseResults(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), SLOT(error(QNetworkReply*)));

}

void YouTubeSearch::error(QNetworkReply *reply) {
    emit error(reply->errorString());
}

void YouTubeSearch::parseResults(QByteArray data) {

    YouTubeStreamReader reader;
    if (!reader.read(data)) {
        qDebug() << "Error parsing XML";
    }
    videos = reader.getVideos();

    foreach (Video *video, videos) {
        // send it to the model
        emit gotVideo(video);
    }

    foreach (Video *video, videos) {
        // preload the thumb
        if (abortFlag) return;
        video->preloadThumbnail();
    }

    emit finished(videos.size());
}

QList<Video*> YouTubeSearch::getResults() {
    return videos;
}

void YouTubeSearch::abort() {
    this->abortFlag = true;
}
