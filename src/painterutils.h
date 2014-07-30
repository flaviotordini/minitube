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

#ifndef PAINTERUTILS_H
#define PAINTERUTILS_H

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

class PainterUtils {

public:
    static void centeredMessage(QString message, QWidget* widget);
    static void topShadow(QWidget *widget);
    static void paintBadge(QPainter *painter, const QString &text, bool center = false);

private:
    PainterUtils();

};

#endif // PAINTERUTILS_H
