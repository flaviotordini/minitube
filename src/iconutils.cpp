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
#include "mainwindow.h"
#include <QAction>

namespace {

void addIconFile(QIcon &icon,
                 const QString &filename,
                 int size,
                 QIcon::Mode mode = QIcon::Normal,
                 QIcon::State state = QIcon::Off) {
    if (QFile::exists(filename)) {
        icon.addFile(filename, QSize(size, size), mode, state);
    }
}
} // namespace

QIcon IconUtils::fromTheme(const QString &name) {
    static const QLatin1String symbolic("-symbolic");
    if (name.endsWith(symbolic)) return QIcon::fromTheme(name);
    QIcon icon = QIcon::fromTheme(name + symbolic);
    if (icon.isNull()) return QIcon::fromTheme(name);
    return icon;
}

QIcon IconUtils::fromResources(const char *name) {
    qDebug() << "Creating icon" << name;
    static const QLatin1String normal("_normal");
    static const QLatin1String active("_active");
    static const QLatin1String selected("_selected");
    static const QLatin1String disabled("_disabled");
    static const QLatin1String checked("_checked");
    static const QLatin1String ext(".png");

    QString path = QStringLiteral(":/icons/");

    if (MainWindow::instance()->palette().window().color().value() > 128)
        path += QLatin1String("light/");
    else
        path += QLatin1String("dark/");

    QIcon icon;

    // WARN keep these sizes updated with what we really use
    for (int size : {16, 24, 32, 88}) {
        const QString pathAndName =
                path + QString::number(size) + QLatin1Char('/') + QLatin1String(name);
        QString iconFilename = pathAndName + normal + ext;
        if (QFile::exists(iconFilename)) {
            addIconFile(icon, iconFilename, size);
            addIconFile(icon, pathAndName + active + ext, size, QIcon::Active);
            addIconFile(icon, pathAndName + selected + ext, size, QIcon::Selected);
            addIconFile(icon, pathAndName + disabled + ext, size, QIcon::Disabled);
            addIconFile(icon, pathAndName + checked + ext, size, QIcon::Normal, QIcon::On);
        }
    }
    return icon;
}

QIcon IconUtils::icon(const char *name) {
    static QMap<QByteArray, QIcon> cache = [] {
        qDebug() << "Init icon cache";
        QMap<QByteArray, QIcon> c;
        QObject::connect(qApp, &QApplication::paletteChanged, qApp, [&c]() {
            qDebug() << "Clearing icon cache";
            c.clear();
        });
        return c;
    }();

    auto i = cache.constFind(QByteArray::fromRawData(name, strlen(name)));
    if (i != cache.constEnd()) return i.value();

    QIcon icon;
#ifdef APP_UBUNTU_NO
    icon = fromTheme(name);
    if (icon.isNull()) icon = fromResources(name);
#else
    icon = fromResources(name);
#endif

    cache.insert(QByteArray(name), icon);
    return icon;
}

QIcon IconUtils::icon(const QVector<const char *> &names) {
    QIcon icon;
    for (auto name : names) {
        icon = IconUtils::icon(name);
        if (!icon.availableSizes().isEmpty()) break;
    }
    return icon;
}

QPixmap IconUtils::iconPixmap(const char *name,
                              int size,
                              const QColor &background,
                              const qreal pixelRatio) {
    QString path = QStringLiteral(":/icons/");
    if (background.value() > 128)
        path += QLatin1String("light/");
    else
        path += QLatin1String("dark/");
    path += QString::number(size) + QLatin1Char('/') + QLatin1String(name) +
            QLatin1String("_normal.png");
    return IconUtils::pixmap(path, pixelRatio);
}

QIcon IconUtils::tintedIcon(const char *name, const QColor &color, const QVector<QSize> &sizes) {
    QIcon i = IconUtils::icon(name);
    QIcon t;
    // if (sizes.isEmpty()) sizes = i.availableSizes();
    for (const QSize &size : sizes) {
        QPixmap pixmap = i.pixmap(size);
        tint(pixmap, color);
        t.addPixmap(pixmap);
    }
    return t;
}

QIcon IconUtils::tintedIcon(const char *name, const QColor &color, const QSize &size) {
    return IconUtils::tintedIcon(name, color, QVector<QSize>() << size);
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

QImage IconUtils::tinted(const QImage &image, const QColor &color, QPainter::CompositionMode mode) {
    QImage img(image.size(), QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&img);
    painter.drawImage(0, 0, grayscaled(image));
    painter.setCompositionMode(mode);
    painter.fillRect(img.rect(), color);
    painter.end();
    img.setAlphaChannel(image.alphaChannel());
    return img;
}

void IconUtils::tint(QPixmap &pixmap, const QColor &color, QPainter::CompositionMode mode) {
    QPainter painter(&pixmap);
    painter.setCompositionMode(mode);
    painter.fillRect(pixmap.rect(), color);
}

QPixmap IconUtils::pixmap(const char *name, const qreal pixelRatio) {
    return pixmap(QString::fromLatin1(name), pixelRatio);
}

QPixmap IconUtils::pixmap(const QString &filename, const qreal pixelRatio) {
    // Check if a "@2x" file exists
    if (pixelRatio > 1.0) {
        int dotIndex = filename.lastIndexOf(QLatin1Char('.'));
        if (dotIndex != -1) {
            QString at2xfileName = filename;
            at2xfileName.insert(dotIndex, QLatin1String("@2x"));
            if (QFile::exists(at2xfileName)) {
                QPixmap pixmap(at2xfileName);
                pixmap.setDevicePixelRatio(pixelRatio);
                return pixmap;
            }
        }
    }
    return QPixmap(filename);
}
