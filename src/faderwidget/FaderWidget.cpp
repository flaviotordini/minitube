#include "FaderWidget.h"

// http://labs.trolltech.com/blogs/2007/08/21/fade-effects-a-blast-from-the-past/

FaderWidget::FaderWidget(QWidget *parent) : QWidget(parent) {
    timeLine = new QTimeLine(250, this);
    timeLine->setFrameRange(1000, 0);
    connect(timeLine, SIGNAL(frameChanged(int)), this, SLOT(update()));
    setAttribute(Qt::WA_DeleteOnClose);
    resize(parent->size());
}

void FaderWidget::start(QPixmap frozenView) {
    this->frozenView = frozenView;
    timeLine->start();
    show();
}

void FaderWidget::paintEvent(QPaintEvent *) {
    const qreal opacity = timeLine->currentFrame() / 1000.;
    QPainter painter(this);
    painter.setOpacity(opacity);
    painter.drawPixmap(0, 0, frozenView);
    // qDebug() << opacity;

    if (opacity <= 0.)
        close();

}
