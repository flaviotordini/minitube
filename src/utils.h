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

#ifndef UTILS_H
#define UTILS_H

#include <QtGui>

class Utils {

public:
    static QIcon themeIcon(const QString &name);
    static QIcon icon(const QString &name);
    static QIcon icon(const QStringList &names);
    static QIcon tintedIcon(const QString &name, const QColor &color,
                            QList<QSize> sizes = QList<QSize>());
    static void setupAction(QAction *action);

private:
    Utils() { }
    static QImage grayscaled(const QImage &image);
    static QImage tinted(const QImage &image, const QColor &color,
                         QPainter::CompositionMode mode = QPainter::CompositionMode_Screen);
};

#endif // UTILS_H
