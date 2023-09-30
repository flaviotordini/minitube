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

#include "refinesearchbutton.h"
#include "iconutils.h"

RefineSearchButton::RefineSearchButton(QWidget *parent) : QPushButton(parent) {
    hovered = false;

    const int refineButtonSize = 48;
    setMinimumSize(refineButtonSize, refineButtonSize);
    setMaximumSize(refineButtonSize, refineButtonSize);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void RefineSearchButton::paintEvent(QPaintEvent *) {
    QColor backgroundColor = palette().windowText().color();
    backgroundColor.setAlpha(hovered ? 192 : 170);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(backgroundColor);
    painter.drawEllipse(QPoint(width(), height()), width() - 2, height() - 2);

    QPixmap pixmap = IconUtils::icon("edit-find", backgroundColor).pixmap(24);
    int pw = pixmap.width() / pixmap.devicePixelRatio();
    int ph = pixmap.height() / pixmap.devicePixelRatio();
    painter.drawPixmap(width() - pw - 6, height() - ph - 6, pw, ph, pixmap);
}

void RefineSearchButton::enterEvent(QEvent *) {
    hovered = true;
    update();
}

void RefineSearchButton::leaveEvent(QEvent *) {
    hovered = false;
    update();
}
