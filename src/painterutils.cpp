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

void PainterUtils::paintBadge(QPainter *painter, const QString &text, bool center) {
    static const QPixmap badge1 = QPixmap(":/images/badge.png");
    static const QPixmap badge3 = QPixmap(":/images/badge3.png");
    static const QPixmap badge4 = QPixmap(":/images/badge4.png");
    static const int size = badge1.height();

    const int textSize = text.size();

    QPixmap badge;
    if (textSize < 3) badge = badge1;
    else if (textSize == 3) badge = badge3;
    else badge = badge4;

    int x = 0;
    if (center) x -= badge.width() / 2;

    QRect rect(x, 0, badge.width(), size);
    painter->drawPixmap(rect, badge);

    QFont f = painter->font();
    f.setPixelSize(11);
    f.setHintingPreference(QFont::PreferNoHinting);
#ifdef APP_MAC
    f.setFamily("Helvetica");
#endif
#ifdef APP_WIN
    rect.adjust(0, -2, 0, 0);
#endif
#ifdef Q_OS_LINUX
    rect.adjust(0, -1, 0, 0);
#endif
    painter->save();
    painter->setFont(f);

    rect.adjust(0, 1, 0, 0);
    painter->setPen(QColor(0, 0, 0, 64));
    painter->drawText(rect, Qt::AlignCenter, text);

    rect.adjust(0, -1, 0, 0);
    painter->setPen(Qt::white);
    painter->drawText(rect, Qt::AlignCenter, text);

    painter->restore();
}
