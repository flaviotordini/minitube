/* $BEGIN_LICENSE

This file is part of Minitube.
Copyright 2009, Flavio Tordini <flavio.tordini@gmail.com>

Minitube is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Minitube is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Minitube.  If not, see <http://www.gnu.org/licenses/>.

$END_LICENSE */

#include "video.h"
#include "datautils.h"
#include "http.h"
#include "httputils.h"
#include "jsfunctions.h"
#include "playlistitemdelegate.h"
#include "videodefinition.h"

#include "ytjsvideo.h"
#include "ytvideo.h"

#include "variantpromise.h"

Video::Video()
    : duration(0), viewCount(-1), license(LicenseYouTube), definitionCode(0), ytVideo(nullptr),
      ytjsVideo(nullptr) {}

Video::~Video() {
    qDebug() << "Deleting" << id;
}

Video *Video::clone() {
    Video *clone = new Video();
    clone->title = title;
    clone->description = description;
    clone->channelTitle = channelTitle;
    clone->channelId = channelId;
    clone->webpage = webpage;
    clone->streamUrl = streamUrl;
    clone->thumbs = thumbs;
    clone->thumbsNeedSorting = thumbsNeedSorting;
    clone->duration = duration;
    clone->formattedDuration = formattedDuration;
    clone->published = published;
    clone->formattedPublished = formattedPublished;
    clone->viewCount = viewCount;
    clone->formattedViewCount = formattedViewCount;
    clone->id = id;
    clone->definitionCode = definitionCode;
    return clone;
}

const QString &Video::getWebpage() {
    if (webpage.isEmpty() && !id.isEmpty())
        webpage.append("https://www.youtube.com/watch?v=").append(id);
    return webpage;
}

void Video::setWebpage(const QString &value) {
    webpage = value;

    // Get Video ID
    if (id.isEmpty()) {
        QRegExp re(JsFunctions::instance()->videoIdRE());
        if (re.indexIn(webpage) == -1) {
            qWarning() << QString("Cannot get video id for %1").arg(webpage);
            // emit errorStreamUrl(QString("Cannot get video id for %1").arg(m_webpage.toString()));
            // loadingStreamUrl = false;
            return;
        }
        id = re.cap(1);
    }
}

void Video::setDuration(int value) {
    duration = value;
    formattedDuration = DataUtils::formatDuration(duration);
}

void Video::setViewCount(int value) {
    viewCount = value;
    formattedViewCount = DataUtils::formatCount(viewCount);
}

void Video::setPublished(const QDateTime &value) {
    published = value;
    formattedPublished = DataUtils::formatDateTime(published);
}

void Video::streamUrlLoaded(const QString &streamUrl, const QString &audioUrl) {
    qDebug() << "Streams loaded";
    this->streamUrl = streamUrl;
    emit gotStreamUrl(streamUrl, audioUrl);
    if (ytVideo) {
        definitionCode = ytVideo->getDefinitionCode();
        ytVideo->deleteLater();
        ytVideo = nullptr;
    }
    if (ytjsVideo) {
        definitionCode = ytjsVideo->getDefinitionCode();
        ytjsVideo->deleteLater();
        ytjsVideo = nullptr;
    }
}

void Video::loadStreamUrlJS() {
    if (ytjsVideo) {
        qDebug() << "Already loading" << id;
        return;
    }
    ytjsVideo = new YTJSVideo(id, this);
    connect(ytjsVideo, &YTJSVideo::gotStreamUrl, this, &Video::streamUrlLoaded);
    connect(ytjsVideo, &YTJSVideo::errorStreamUrl, this, [this](const QString &msg) {
        qDebug() << msg;
        ytjsVideo->deleteLater();
        ytjsVideo = nullptr;
        // loadStreamUrlYT();
        emit errorStreamUrl(msg);        
    });
    ytjsVideo->loadStreamUrl();
}

void Video::loadStreamUrlYT() {
    if (ytVideo) {
        qDebug() << "Already loading" << id;
        return;
    }
    ytVideo = new YTVideo(id, this);
    connect(ytVideo, &YTVideo::gotStreamUrl, this, &Video::streamUrlLoaded);
    connect(ytVideo, &YTVideo::errorStreamUrl, this, [this](const QString &msg) {
        qDebug() << msg;
        emit errorStreamUrl(msg);
        ytVideo->deleteLater();
        ytVideo = nullptr;
    });
    ytVideo->loadStreamUrl();
}

void Video::loadStreamUrl() {
    loadStreamUrlJS();
}

bool Video::isLoadingStreamUrl() const {
    return ytVideo != nullptr || ytjsVideo != nullptr;
}

void Video::abortLoadStreamUrl() {
    if (ytVideo) {
        ytVideo->disconnect(this);
        ytVideo->deleteLater();
        ytVideo = nullptr;
    }
    if (ytjsVideo) {
        ytjsVideo->disconnect(this);
        ytjsVideo->deleteLater();
        ytjsVideo = nullptr;
    }
}

void Video::addThumb(int width, int height, QString url) {
    thumbs << YTThumb(width, height, url);
    thumbsNeedSorting = true;
}

VariantPromise &Video::loadThumb(QSize size, qreal pixelRatio) {
    if (thumbsNeedSorting) {
        std::sort(thumbs.begin(), thumbs.end(),
                  [](auto a, auto b) { return a.getWidth() < b.getWidth(); });
        thumbsNeedSorting = false;
    }

    auto promise = new VariantPromise(this);
    if (thumbs.isEmpty()) {
        QTimer::singleShot(0, promise, [promise] { promise->reject("Empty thumbs"); });
        return *promise;
    }

    auto reallyLoad = [this, promise, size, pixelRatio](auto &&self,
                                                        YTThumb *previous = nullptr) -> void {
        YTThumb *selected = nullptr;

        static bool fallback = false;
        if (fallback) {
            qDebug() << "Doing fallback loop";
            bool skip = previous != nullptr;
            for (int i = thumbs.size() - 1; i >= 0; --i) {
                auto &thumb = thumbs.at(i);
                if (!skip) {
                    selected = (YTThumb *)&thumb;
                    qDebug() << "selected" << selected->getUrl();
                    break;
                }
                if (&thumb == previous) skip = false;
            }
        } else {
            bool skip = previous != nullptr;
            for (auto &thumb : qAsConst(thumbs)) {
                if (!skip && thumb.getWidth() * pixelRatio >= size.width() &&
                    thumb.getHeight() * pixelRatio >= size.height()) {
                    selected = (YTThumb *)&thumb;
                    qDebug() << "selected" << selected->getUrl();
                    break;
                }
                if (&thumb == previous) skip = false;
            }
        }
        if (!selected && !fallback) {
            qDebug() << "Falling back";
            fallback = true;
            self(self);
            return;
        }
        if (selected) {
            qDebug() << "Loading" << selected->getUrl();
            selected->load(promise)
                    .then([promise](auto variant) { promise->resolve(variant); })
                    .onFailed([self, selected] { self(self, selected); });
        } else
            promise->reject("No thumb");
    };
    reallyLoad(reallyLoad);

    return *promise;
}
