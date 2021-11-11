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

#include "videosourcewidget.h"
#include "fontutils.h"
#include "http.h"
#include "httputils.h"
#include "iconutils.h"
#include "variantpromise.h"
#include "video.h"
#include "videosource.h"

VideoSourceWidget::VideoSourceWidget(VideoSource *videoSource, QWidget *parent)
    : GridWidget(parent),
      videoSource(videoSource),
      lastPixelRatio(0) {
    videoSource->setParent(this);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(this, SIGNAL(activated()), SLOT(activate()));
}

void VideoSourceWidget::activate() {
    emit activated(videoSource);
}

void VideoSourceWidget::previewVideo(const QVector<Video *> &videos) {
    videoSource->disconnect();
    if (videos.isEmpty()) {
        qDebug() << "Unavailable video source" << videoSource->getName();
        emit unavailable(this);
        return;
    }
    Video *video = videos.at(0);
    lastPixelRatio = devicePixelRatio();

    video->loadThumb(size(), lastPixelRatio)
            .then([this](auto variant) { setPixmapData(variant.toByteArray()); })
            .onFailed([](auto msg) { qDebug() << msg; })
            .finally([this, videos] {
                for (auto v : videos)
                    v->deleteLater();
                emit previewLoaded();
            });
}

void VideoSourceWidget::setPixmapData(const QByteArray &bytes) {
    pixmap.loadFromData(bytes);
    pixmap.setDevicePixelRatio(lastPixelRatio);
    update();
}

EmptyPromise *VideoSourceWidget::loadPreview() {
    auto promise = new EmptyPromise(this);
    connect(this, &VideoSourceWidget::previewLoaded, promise, &EmptyPromise::resolve);
    connect(this, &VideoSourceWidget::unavailable, promise, [promise] {
        promise->reject(staticMetaObject.className() + QLatin1String(" unavailable"));
    });

    connect(videoSource, SIGNAL(gotVideos(QVector<Video *>)), SLOT(previewVideo(QVector<Video *>)),
            Qt::UniqueConnection);
    videoSource->loadVideos(1, 1);

    return promise;
}

QPixmap VideoSourceWidget::playPixmap() {
    const int s = height() / 2;
    const int padding = s / 8;

    qreal ratio = devicePixelRatio();
    QPixmap playIcon = QPixmap(s * ratio, s * ratio);
    playIcon.setDevicePixelRatio(ratio);
    playIcon.fill(Qt::transparent);
    QPainter painter(&playIcon);
    QPolygon polygon;
    polygon << QPoint(padding, padding)
            << QPoint(s - padding, s / 2)
            << QPoint(padding, s - padding);
    painter.setRenderHints(QPainter::Antialiasing, true);

    // QColor color = pressed ? Qt::black : Qt::white;
    QColor color = Qt::white;
    painter.setBrush(color);
    QPen pen;
    pen.setColor(color);
    pen.setWidth(10);
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setCapStyle(Qt::RoundCap);
    painter.setPen(pen);
    painter.drawPolygon(polygon);
    return playIcon;
}

void VideoSourceWidget::paintEvent(QPaintEvent *event) {
    GridWidget::paintEvent(event);
    // if (devicePixelRatio() != lastPixelRatio) loadPreview();

    if (pixmap.isNull()) return;

    QPainter p(this);

    qreal ratio = lastPixelRatio;
    int w = width() * ratio;
    int h = height() * ratio;

    int xOffset = 0;
    int xOrigin = 0;
    int wDiff = pixmap.width() - w;
    if (wDiff > 0) xOffset = wDiff / 2;
    else xOrigin = -wDiff / 2;
    int yOffset = 0;
    int yOrigin = 0;
    int hDiff = pixmap.height() - h;
    if (hDiff > 0) yOffset = hDiff / 2;
    else yOrigin = -hDiff / 2;
    p.drawPixmap(xOrigin, yOrigin, pixmap, xOffset, yOffset, w, h);

    w = width();
    h = height();

    if (hovered) {
        QPixmap play = playPixmap();
        p.save();
        p.setOpacity(.5);
        p.drawPixmap(
                    (w - play.width() * ratio) / 2,
                    (h * 2/3 - play.height() * ratio) / 2,
                    play
                    );
        p.restore();
    }

    QRect nameBox = rect();
    nameBox.adjust(0, 0, 0, -h*2/3);
    nameBox.translate(0, h - nameBox.height());
    p.save();
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 128));
    p.drawRect(nameBox);
    p.restore();

    QString name = videoSource->getName();
    bool tooBig = false;
    p.save();
    p.setFont(FontUtils::big());
    QRect textBox = p.boundingRect(nameBox, Qt::AlignCenter | Qt::TextWordWrap, name);
    if (textBox.height() > nameBox.height()) {
        p.setFont(font());
        textBox = p.boundingRect(nameBox, Qt::AlignCenter | Qt::TextWordWrap, name);
        if (textBox.height() > nameBox.height()) {
            p.setClipRect(nameBox);
            tooBig = true;
        }
    }
    p.setPen(Qt::white);
    if (tooBig)
        p.drawText(nameBox, Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap, name);
    else
        p.drawText(textBox, Qt::AlignCenter | Qt::TextWordWrap, name);
    p.restore();

    if (hasFocus()) {
        p.save();
        QPen pen;
        pen.setBrush(palette().highlight());
        pen.setWidth(2);
        p.setPen(pen);
        p.drawRect(rect());
        p.restore();
    }
}
