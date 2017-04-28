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

    /*
    rect.adjust(0, -1, 0, 0);
    painter.setPen(QColor(0, 0, 0, 128));
    painter.drawText(rect, Qt::AlignCenter, message);
    rect.adjust(0, 1, 0, 0);
    */

    QPen textPen;
    textPen.setBrush(widget->palette().mid());
    painter.setPen(textPen);
    painter.drawText(rect, Qt::AlignCenter, message);
}

void PainterUtils::topShadow(QWidget *widget) {
    static QLinearGradient shadow;
    static const int shadowHeight = 10;
    if (shadow.stops().count() == 2) {
        shadow.setFinalStop(0, shadowHeight);
        const qreal initialOpacity = 96;
        for (qreal i = 0; i <= 1; i += 1.0/shadowHeight) {
            qreal opacity = qPow(initialOpacity, (1.0 - i)) - 1;
            shadow.setColorAt(i, QColor(0x00, 0x00, 0x00, opacity));
        }
    }
    QRect rect = widget->rect();
    QPainter p(widget);
    p.fillRect(rect.x(), rect.y(), rect.width(), shadowHeight, QBrush(shadow));
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
