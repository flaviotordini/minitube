#include "videosourcewidget.h"
#include "videosource.h"
#include "video.h"
#include "fontutils.h"

VideoSourceWidget::VideoSourceWidget(VideoSource *videoSource, QWidget *parent)
    : QWidget(parent),
      videoSource(videoSource),
      hovered(false),
      pressed(false) {

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::StrongFocus);

    connect(videoSource, SIGNAL(gotVideos(QList<Video*>)),
            SLOT(previewVideo(QList<Video*>)), Qt::UniqueConnection);
    videoSource->loadVideos(1, 1);
}

void VideoSourceWidget::activate() {
    emit activated(videoSource);
}

void VideoSourceWidget::previewVideo(QList<Video*> videos) {
    videoSource->disconnect();
    if (videos.isEmpty()) return;
    video = videos.first();
    connect(video, SIGNAL(gotMediumThumbnail(QByteArray)),
            SLOT(setPixmapData(QByteArray)), Qt::UniqueConnection);
    video->loadMediumThumbnail();
}

void VideoSourceWidget::setPixmapData(QByteArray bytes) {
    video->deleteLater();
    video = 0;
    pixmap.loadFromData(bytes);
    update();
}

QPixmap VideoSourceWidget::playPixmap() {
    const int s = height() / 2;
    const int padding = s / 8;
    QPixmap playIcon = QPixmap(s, s);
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

void VideoSourceWidget::paintEvent(QPaintEvent *) {
    if (pixmap.isNull()) return;

    QPainter p(this);

    const int w = width();
    const int h = height();

    int xOffset = 0;
    int xOrigin = 0;
    int wDiff = pixmap.width() - w;
    if (wDiff > 0) xOffset = wDiff / 2;
    else xOrigin = -wDiff / 2;
    int yOffset = 0;
    int yOrigin = 0;
    int hDiff = pixmap.height() - h;
    if (hDiff > 0) yOffset = hDiff / 4;
    else yOrigin = -hDiff / 2;
    p.drawPixmap(xOrigin, yOrigin, pixmap, xOffset, yOffset, w, h);

    if (hovered) {
        QPixmap play = playPixmap();
        p.save();
        p.setOpacity(.5);
        p.drawPixmap(
                    (w - play.width()) / 2,
                    (h * 2/3 - play.height()) / 2,
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
    p.setFont(FontUtils::medium());
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

void VideoSourceWidget::mouseMoveEvent (QMouseEvent *event) {
    QWidget::mouseMoveEvent(event);
    hovered = rect().contains(event->pos());
}

void VideoSourceWidget::mousePressEvent(QMouseEvent *event) {
    QWidget::mousePressEvent(event);
    if (event->button() != Qt::LeftButton) return;
    pressed = true;
    update();
}

void VideoSourceWidget::mouseReleaseEvent(QMouseEvent *event) {
    QWidget::mouseReleaseEvent(event);
    if (event->button() != Qt::LeftButton) return;
    pressed = false;
    if (hovered) emit activated(videoSource);
}

void VideoSourceWidget::leaveEvent(QEvent *event) {
    QWidget::leaveEvent(event);
    hovered = false;
    update();
}

void VideoSourceWidget::enterEvent(QEvent *event) {
    QWidget::enterEvent(event);
    hovered = true;
    update();
}

void VideoSourceWidget::keyReleaseEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Return)
        emit activated(videoSource);
}
