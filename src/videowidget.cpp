#include "videowidget.h"

VideoWidget::VideoWidget(QWidget *parent) : Phonon::VideoWidget(parent) {
    // mouse autohide
    // setMouseTracking(true);
    mouseTimer = new QTimer(this);
    mouseTimer->setInterval(3000);
    mouseTimer->setSingleShot(true);
    connect(mouseTimer, SIGNAL(timeout()), SLOT(hideMouse()));
}

void VideoWidget::mouseMoveEvent(QMouseEvent * /* event */) {
    // qDebug() << "mouseMoveEvent";

    // show the normal cursor
    unsetCursor();

    // then hide it again after a few seconds
    mouseTimer->start();
}

void VideoWidget::hideMouse() {
    // qDebug() << "hideMouse()";
    setCursor(Qt::BlankCursor);
}
