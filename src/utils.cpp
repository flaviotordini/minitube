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

#include "utils.h"
#ifdef APP_EXTRA
#include "extra.h"
#endif

QIcon Utils::themeIcon(const QString &name) {
    if (QIcon::hasThemeIcon(name))
        return QIcon::fromTheme(name);
    else
        return QIcon(QString(":/images/%1.png").arg(name));
}

QIcon Utils::icon(const QString &name) {
#ifdef APP_EXTRA
    return Extra::getIcon(name);
#else
    return themeIcon(name);
#endif
}

QIcon Utils::icon(const QStringList &names) {
    QIcon icon;
    foreach (QString name, names) {
        icon = Utils::themeIcon(name);
        if (!icon.availableSizes().isEmpty()) break;
    }
    return icon;
}

QIcon Utils::tintedIcon(const QString &name, const QColor &color, QList<QSize> sizes) {
    QIcon i = icon(name);
    QIcon t;
    if (sizes.isEmpty()) sizes = i.availableSizes();
    foreach (QSize size, sizes) {
        QPixmap pixmap = i.pixmap(size);
        QImage tintedImage = tinted(pixmap.toImage(), color);
        t.addPixmap(QPixmap::fromImage(tintedImage));
    }
    return t;
}

QImage Utils::grayscaled(const QImage &image) {
    QImage img = image;
    int pixels = img.width() * img.height();
    unsigned int *data = (unsigned int *)img.bits();
    for (int i = 0; i < pixels; ++i) {
        int val = qGray(data[i]);
        data[i] = qRgba(val, val, val, qAlpha(data[i]));
    }
    return img;
}

QImage Utils::tinted(const QImage &image, const QColor &color,
              QPainter::CompositionMode mode) {
    QImage img(image.size(), QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&img);
    painter.drawImage(0, 0, grayscaled(image));
    painter.setCompositionMode(mode);
    painter.fillRect(img.rect(), color);
    painter.end();
    img.setAlphaChannel(image.alphaChannel());
    return img;
}

void Utils::setupAction(QAction *action) {
    // never autorepeat.
    // unexperienced users tend to keep keys pressed for a "long" time
    action->setAutoRepeat(false);

    // show keyboard shortcuts in the status bar
    if (!action->shortcut().isEmpty())
        action->setStatusTip(action->statusTip() +
                             " (" +
                             action->shortcut().toString(QKeySequence::NativeText) +
                            ")");
}
