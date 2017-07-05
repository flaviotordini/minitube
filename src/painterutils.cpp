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

#include "painterutils.h"
#include "fontutils.h"
#include "iconutils.h"

PainterUtils::PainterUtils() { }

void PainterUtils::centeredMessage(const QString &message, QWidget* widget) {
    QPainter painter(widget);
    painter.setFont(FontUtils::big());
    QSize textSize(QFontMetrics(painter.font()).size(Qt::TextSingleLine, message));
    QPoint topLeft(
                (widget->width()-textSize.width())/2,
                ((widget->height()-textSize.height())/2)
                );
    QRect rect(topLeft, textSize);
    painter.setOpacity(.5);
    painter.drawText(rect, Qt::AlignCenter, message);
}

void PainterUtils::paintBadge(QPainter *painter, const QString &text, bool center, QColor backgroundColor) {
    painter->save();

    QRect textBox = painter->boundingRect(QRect(), Qt::AlignCenter, text);
    int w = textBox.width() + painter->fontMetrics().width('m');
    int x = 0;
    if (center) x -= w / 2;
    QRect rect(x, 0, w, textBox.height());
    if (rect.width() < rect.height() || text.length() == 1) rect.setWidth(rect.height());

    painter->setPen(Qt::NoPen);
    painter->setBrush(backgroundColor);
    painter->setRenderHint(QPainter::Antialiasing);
    qreal borderRadius = rect.height()/2.;
    painter->drawRoundedRect(rect, borderRadius, borderRadius);

    painter->setPen(Qt::white);
    painter->drawText(rect, Qt::AlignCenter, text);

    painter->restore();
}
