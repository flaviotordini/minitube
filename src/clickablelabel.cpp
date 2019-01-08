#include "clickablelabel.h"

ClickableLabel::ClickableLabel(QWidget *parent) : QLabel(parent) {
    setCursor(Qt::PointingHandCursor);
}

ClickableLabel::ClickableLabel(const QString &text, QWidget *parent) : QLabel(text, parent) {
    setCursor(Qt::PointingHandCursor);
}

void ClickableLabel::mouseReleaseEvent(QMouseEvent *e) {
    if (rect().contains(e->pos())) emit clicked();
}

void ClickableLabel::leaveEvent(QEvent *event) {
    QLabel::leaveEvent(event);
    emit hovered(false);
}

void ClickableLabel::enterEvent(QEvent *event) {
    QLabel::enterEvent(event);
    emit hovered(true);
}
