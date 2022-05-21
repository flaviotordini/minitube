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
    static void setSizes(const QVector<int> &value);

    static QIcon fromTheme(const QString &name);
    static QIcon fromResources(const char *name);

    template <class T> static void setIcon(T *obj, const char *name) {
        QIcon i = icon(name);
        obj->setIcon(i);
        obj->connect(qApp, &QGuiApplication::paletteChanged, obj, [obj, name] {
            qDebug() << "Updating icon" << name;
            QIcon i = icon(name);
            obj->setIcon(i);
        });
    }
    static QIcon icon(const char *name);
    static QIcon icon(const QVector<const char *> &names);

    static QPixmap
    iconPixmap(const char *name, int size, const QColor &background, const qreal pixelRatio);

    static QIcon tintedIcon(const char *name, const QColor &color, const QVector<QSize> &sizes);
    static QIcon tintedIcon(const char *name, const QColor &color, const QSize &size);

    // HiDPI stuff
    static QPixmap pixmap(const char *name, const qreal pixelRatio);
    static QPixmap pixmap(const QString &filename, const qreal pixelRatio);

    static void tint(QPixmap &pixmap,
                     const QColor &color,
                     QPainter::CompositionMode mode = QPainter::CompositionMode_SourceIn);

private:
    IconUtils() {}
    static QImage grayscaled(const QImage &image);
};

#endif // ICONUTILS_H
