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

SnapshotPreview::SnapshotPreview(QWidget *parent) : QWidget(parent),
#ifdef APP_PHONON
    mediaObject(0),
    audioOutput(0)
#endif
    {
    setAttribute(Qt::WA_ShowWithoutActivating);
    setWindowFlags(Qt::FramelessWindowHint);
#if QT_VERSION >= 0x050000
    setWindowFlags(Qt::WindowDoesNotAcceptFocus);
#endif

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
#ifdef APP_PHONON
    if (!mediaObject) {
        mediaObject = new Phonon::MediaObject(this);
        audioOutput = new Phonon::AudioOutput(Phonon::NotificationCategory, this);
        Phonon::createPath(mediaObject, audioOutput);
    }
    mediaObject->setCurrentSource(QUrl("qrc:///sounds/snapshot.wav"));
    mediaObject->play();
#endif
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
    if (isVisible()) update();
    else show();
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
