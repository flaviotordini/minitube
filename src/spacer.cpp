#include "spacer.h"

Spacer::Spacer(QWidget *parent) : QWidget(parent) { }

QSize Spacer::sizeHint() const {
    return QSize(10, 1);
}
