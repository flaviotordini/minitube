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

#include "gridwidget.h"

GridWidget::GridWidget(QWidget *parent) :
    QWidget(parent),
    hovered(false),
    pressed(false) {
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::StrongFocus);
}

void GridWidget::mouseMoveEvent(QMouseEvent *event) {
    QWidget::mouseMoveEvent(event);
    hovered = rect().contains(event->pos());
}

void GridWidget::mousePressEvent(QMouseEvent *event) {
    QWidget::mousePressEvent(event);
    if (event->button() != Qt::LeftButton) return;
    pressed = true;
    update();
}

void GridWidget::mouseReleaseEvent(QMouseEvent *event) {
    QWidget::mouseReleaseEvent(event);
    if (event->button() != Qt::LeftButton) return;
    pressed = false;
    if (hovered) emit activated();
}

void GridWidget::leaveEvent(QEvent *event) {
    QWidget::leaveEvent(event);
    hovered = false;
    update();
}

void GridWidget::enterEvent(QEvent *event) {
    QWidget::enterEvent(event);
    hovered = true;
    update();
}

void GridWidget::keyReleaseEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Return)
        emit activated();
}
