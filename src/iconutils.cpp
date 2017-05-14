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

#include "iconutils.h"
#include <QAction>
#include "mainwindow.h"

QIcon IconUtils::fromTheme(const QString &name) {
    const QLatin1String symbolic("-symbolic");
    if (name.endsWith(symbolic)) return QIcon::fromTheme(name);
    QIcon icon = QIcon::fromTheme(name + symbolic);
    if (icon.isNull()) return QIcon::fromTheme(name);
    return icon;
}

QIcon IconUtils::fromResources(const QString &name) {
    QIcon icon = QIcon(QString(":/images/%1.png").arg(name));
    if (!icon.isNull()) {
        icon.addPixmap(QPixmap(QString(":/images/%1_active.png").arg(name)), QIcon::Active);
        icon.addPixmap(QPixmap(QString(":/images/%1_selected.png").arg(name)), QIcon::Selected);
        icon.addPixmap(QPixmap(QString(":/images/%1_disabled.png").arg(name)), QIcon::Disabled);
        icon.addPixmap(QPixmap(QString(":/images/%1_checked.png").arg(name)), QIcon::Normal, QIcon::On);

        icon.addPixmap(QPixmap(QString(":/images/%1_active@2x.png").arg(name)), QIcon::Active);
        icon.addPixmap(QPixmap(QString(":/images/%1_selected@2x.png").arg(name)), QIcon::Selected);
        icon.addPixmap(QPixmap(QString(":/images/%1_disabled@2x.png").arg(name)), QIcon::Disabled);
        icon.addPixmap(QPixmap(QString(":/images/%1_checked@2x.png").arg(name)), QIcon::Normal, QIcon::On);
    }
    return icon;
}

QIcon IconUtils::icon(const QString &name) {
#ifdef APP_LINUX
    QIcon icon = fromTheme(name);
    if (icon.isNull()) icon = fromResources(name);
    return icon;
#else
    return fromResources(name);
#endif
}

QIcon IconUtils::icon(const QStringList &names) {
    QIcon icon;
    foreach (const QString &name, names) {
        icon = IconUtils::icon(name);
        if (!icon.availableSizes().isEmpty()) break;
    }
    return icon;
}

QIcon IconUtils::tintedIcon(const QString &name, const QColor &color, QList<QSize> sizes) {
    QIcon i = IconUtils::icon(name);
    QIcon t;
    if (sizes.isEmpty()) sizes = i.availableSizes();
    foreach (const QSize &size, sizes) {
        QPixmap pixmap = i.pixmap(size);
        QImage tintedImage = tinted(pixmap.toImage(), color);
        t.addPixmap(QPixmap::fromImage(tintedImage));
    }
    return t;
}

QIcon IconUtils::tintedIcon(const QString &name, const QColor &color, const QSize &size) {
    return IconUtils::tintedIcon(name, color, QList<QSize>() << size);
}

QImage IconUtils::grayscaled(const QImage &image) {
    QImage img = image;
    int pixels = img.width() * img.height();
    unsigned int *data = (unsigned int *)img.bits();
    for (int i = 0; i < pixels; ++i) {
        int val = qGray(data[i]);
        data[i] = qRgba(val, val, val, qAlpha(data[i]));
    }
    return img;
}

QImage IconUtils::tinted(const QImage &image, const QColor &color,
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

void IconUtils::setupAction(QAction *action) {
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

QPixmap IconUtils::pixmap(const QString &name) {
    // Check if a "@2x" file exists
    const qreal pixelRatio = IconUtils::pixelRatio();
    QString fileName = name;
    if (pixelRatio > 1.0) {
        int dotIndex = fileName.lastIndexOf(QLatin1Char('.'));
        if (dotIndex != -1) {
            QString at2xfileName = fileName;
            at2xfileName.insert(dotIndex, QStringLiteral("@2x"));
            if (QFile::exists(at2xfileName)) {
                fileName = at2xfileName;
                QPixmap pixmap(fileName);
                pixmap.setDevicePixelRatio(pixelRatio);
                return pixmap;
            }
        }
    }

    return QPixmap(fileName);
}

qreal IconUtils::pixelRatio() {
#if QT_VERSION >= 0x050600
    return MainWindow::instance()->devicePixelRatioF();
#else
    return MainWindow::instance()->devicePixelRatio();
#endif
}
