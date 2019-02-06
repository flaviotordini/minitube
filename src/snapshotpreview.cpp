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

#include "snapshotpreview.h"
#include "mainwindow.h"

#ifdef MEDIA_QTAV
#include "mediaqtav.h"
#endif
#ifdef MEDIA_MPV
#include "mediampv.h"
#endif

SnapshotPreview::SnapshotPreview(QWidget *parent) : QWidget(parent), mediaObject(0) {
    setAttribute(Qt::WA_ShowWithoutActivating);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus);
    setAttribute(Qt::WA_StaticContents);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    setEnabled(false);
    setFocusPolicy(Qt::NoFocus);

    timeLine = new QTimeLine(300, this);
    timeLine->setCurveShape(QTimeLine::LinearCurve);
    timeLine->setFrameRange(0, 20);
    connect(timeLine, SIGNAL(frameChanged(int)), SLOT(update()));

    timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(1500);
    connect(timer, SIGNAL(timeout()), SLOT(finish()));

    hide();
}

void SnapshotPreview::start(QWidget *widget, const QPixmap &pixmap, bool soundOnly) {
    if (!mediaObject) {
#ifdef MEDIA_QTAV
        mediaObject = new MediaQtAV(this);
#elif defined MEDIA_MPV
        // mediaObject = new MediaMPV(this);
#else
        qFatal("No media backend defined");
#endif
        if (mediaObject) {
            mediaObject->setAudioOnly(true);
            mediaObject->init();
        }
    }

    if (mediaObject) mediaObject->play("qrc:///sounds/snapshot.wav");
    if (soundOnly) return;

    resize(pixmap.size());
#if defined(APP_MAC) || defined(APP_WIN)
    QPoint pos = widget->mapToGlobal(widget->pos());
    pos.setY(pos.y() + ((widget->height() - pixmap.height()) / 2));
    pos.setX(pos.x() + ((widget->width() - pixmap.width()) / 2));
    move(pos);
#else
    QPoint pos;
    pos.setY((widget->height() - pixmap.height()) / 2);
    pos.setX((widget->width() - pixmap.width()) / 2);
    move(pos);
    setParent(widget);
#endif

    this->pixmap = pixmap;
#ifndef APP_MAC
    timeLine->start();
#endif
    timer->start();
    if (isVisible())
        update();
    else
        show();
}

void SnapshotPreview::paintEvent(QPaintEvent *e) {
    Q_UNUSED(e);
    QPainter p(this);
    if (timeLine->state() == QTimeLine::Running) {
        p.fillRect(rect(), Qt::white);
        const qreal opacity = timeLine->currentFrame() / 20.;
        p.setOpacity(opacity);
    }
    p.drawPixmap(0, 0, pixmap);
}

void SnapshotPreview::finish() {
    hide();
    emit done();
}
