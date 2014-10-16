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

#ifndef ICONUTILS_H
#define ICONUTILS_H

#include <QtGui>

class IconUtils {

public:
    static QIcon fromTheme(const QString &name);
    static QIcon fromResources(const QString &name);
    static QIcon icon(const QString &name);
    static QIcon icon(const QStringList &names);
    static QIcon tintedIcon(const QString &name, const QColor &color,
                            QList<QSize> sizes = QList<QSize>());
    static QIcon tintedIcon(const QString &name, const QColor &color, const QSize &size);
    static void setupAction(QAction *action);

private:
    IconUtils() { }
    static QImage grayscaled(const QImage &image);
    static QImage tinted(const QImage &image, const QColor &color,
                         QPainter::CompositionMode mode = QPainter::CompositionMode_Screen);
};

#endif // ICONUTILS_H
