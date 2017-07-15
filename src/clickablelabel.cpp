#include "clickablelabel.h"

ClickableLabel::ClickableLabel(QWidget* parent, Qt::WindowFlags f)
    : QLabel(parent) {
    setCursor(Qt::PointingHandCursor);
}

void ClickableLabel::mouseReleaseEvent(QMouseEvent* event) {
    emit clicked();
}
