#include "clickablelabel.h"

ClickableLabel::ClickableLabel(QWidget *parent) : QLabel(parent) {
    setCursor(Qt::PointingHandCursor);
}

ClickableLabel::ClickableLabel(const QString &text, QWidget *parent) : QLabel(text, parent) {
    setCursor(Qt::PointingHandCursor);
}

void ClickableLabel::mousePressEvent(QMouseEvent *e) {
    if (rect().contains(e->pos())) emit clicked();
}

void ClickableLabel::leaveEvent(QEvent *e) {
    emit hovered(false);
    QLabel::leaveEvent(e);
}

void ClickableLabel::enterEvent(QEvent *e) {
    emit hovered(true);
    QLabel::enterEvent(e);
}
