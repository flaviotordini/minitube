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

static const int refineButtonSize = 48;

RefineSearchButton::RefineSearchButton(QWidget *parent) :
    QPushButton(parent) {

    hovered = false;

    setMinimumSize(refineButtonSize, refineButtonSize);
    setMaximumSize(refineButtonSize, refineButtonSize);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setStyleSheet(
                "background: red url(:/images/refine-search.png) no-repeat center;"
                "border: 0;"
                );
}

void RefineSearchButton::paintBackground() const {

}

void RefineSearchButton::paintEvent(QPaintEvent *) {
    // QPushButton::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing, true);
    painter.setBrush(QColor(0,0,0, hovered ? 192 : 170));
    QPen pen(Qt::white);
    pen.setWidth(2);
    painter.setPen(pen);
    painter.drawEllipse(QPoint(width(), height()), width()-2, height()-2);

    QPixmap icon = QPixmap(":/images/refine-search.png");
    painter.drawPixmap(width() - icon.width() - 6, height() - icon.height() - 6,
                       icon.width(), icon.height(),
                       icon);
}

void RefineSearchButton::enterEvent(QEvent *) {
    hovered = true;
    update();
}

void RefineSearchButton::leaveEvent(QEvent *) {
    hovered = false;
    update();
}
