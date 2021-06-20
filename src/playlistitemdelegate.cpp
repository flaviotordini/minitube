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

#include "playlistitemdelegate.h"
#include "datautils.h"
#include "downloaditem.h"
#include "fontutils.h"
#include "iconutils.h"
#include "playlistmodel.h"
#include "playlistview.h"
#include "variantpromise.h"
#include "video.h"
#include "videodefinition.h"

const int PlaylistItemDelegate::thumbHeight = 90;
const int PlaylistItemDelegate::thumbWidth = 160;
const int PlaylistItemDelegate::padding = 8;

namespace {

bool drawElidedText(QPainter *painter, const QRect &textBox, const int flags, const QString &text) {
    QString elidedText =
            painter->fontMetrics().elidedText(text, Qt::ElideRight, textBox.width(), flags);
    painter->drawText(textBox, 0, elidedText);
    return elidedText.length() < text.length();
}
} // namespace

PlaylistItemDelegate::PlaylistItemDelegate(QObject *parent, bool downloadInfo)
    : QStyledItemDelegate(parent), downloadInfo(downloadInfo), progressBar(nullptr) {
    listView = qobject_cast<PlaylistView *>(parent);

    smallerBoldFont = FontUtils::smallBold();
    smallerFont = FontUtils::small();

    if (downloadInfo) {
        progressBar = new QProgressBar(qApp->activeWindow());
        QPalette palette = progressBar->palette();
        palette.setColor(QPalette::Window, Qt::transparent);
        progressBar->setPalette(palette);
        progressBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        progressBar->hide();
    } else
        createPlayIcon();
}

PlaylistItemDelegate::~PlaylistItemDelegate() {
    if (progressBar) delete progressBar;
}

void PlaylistItemDelegate::createPlayIcon() {
    qreal maxRatio = 2.0;
    playIcon = QPixmap(thumbWidth * maxRatio, thumbHeight * maxRatio);
    playIcon.setDevicePixelRatio(maxRatio);
    playIcon.fill(Qt::transparent);

    QPixmap tempPixmap(thumbWidth * maxRatio, thumbHeight * maxRatio);
    tempPixmap.setDevicePixelRatio(maxRatio);
    tempPixmap.fill(Qt::transparent);
    QPainter painter(&tempPixmap);
    painter.setRenderHints(QPainter::Antialiasing, true);

    const int hPadding = padding * 6;
    const int vPadding = padding * 2;

    QPolygon polygon;
    polygon << QPoint(hPadding, vPadding) << QPoint(thumbWidth - hPadding, thumbHeight / 2)
            << QPoint(hPadding, thumbHeight - vPadding);
    painter.setBrush(Qt::white);
    QPen pen;
    pen.setColor(Qt::white);
    pen.setWidth(padding);
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setCapStyle(Qt::RoundCap);
    painter.setPen(pen);
    painter.drawPolygon(polygon);
    painter.end();

    QPainter painter2(&playIcon);
    painter2.setOpacity(.75);
    painter2.drawPixmap(0, 0, tempPixmap);
}

QSize PlaylistItemDelegate::sizeHint(const QStyleOptionViewItem & /*option*/,
                                     const QModelIndex & /*index*/) const {
    return QSize(thumbWidth, thumbHeight);
}

void PlaylistItemDelegate::paint(QPainter *painter,
                                 const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const {
    int itemType = index.data(ItemTypeRole).toInt();
    if (itemType == ItemTypeVideo)
        paintBody(painter, option, index);
    else
        QStyledItemDelegate::paint(painter, option, index);
}

void PlaylistItemDelegate::paintBody(QPainter *painter,
                                     const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const {
    const bool isSelected = option.state & QStyle::State_Selected;
    if (isSelected)
        QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter);

    painter->save();
    painter->translate(option.rect.topLeft());

    QRect line(0, 0, option.rect.width(), option.rect.height());
    if (downloadInfo) line.setWidth(line.width() / 2);

    const bool isActive = index.data(ActiveTrackRole).toBool();

    // get the video metadata
    Video *video = index.data(VideoRole).value<VideoPointer>().data();

    // draw the "current track" highlight underneath the text
    if (isActive && !isSelected) paintActiveOverlay(painter, option, line);

    // thumb
    qreal pixelRatio = painter->device()->devicePixelRatioF();
    QByteArray thumbKey = ("t" + QString::number(pixelRatio)).toUtf8();
    const QPixmap &thumb = video->property(thumbKey).value<QPixmap>();
    if (!thumb.isNull()) {
        painter->drawPixmap(0, 0, thumb);
        if (video->getDuration() > 0) drawTime(painter, video->getFormattedDuration(), line);
    } else
        video->loadThumb({thumbWidth, thumbHeight}, pixelRatio)
                .then([pixelRatio, thumbKey, video](auto variant) {
                    QPixmap pixmap;
                    pixmap.loadFromData(variant.toByteArray());
                    pixmap.setDevicePixelRatio(pixelRatio);
                    const int thumbWidth = PlaylistItemDelegate::thumbWidth * pixelRatio;
                    if (pixmap.width() > thumbWidth)
                        pixmap = pixmap.scaledToWidth(thumbWidth, Qt::SmoothTransformation);
                    video->setProperty(thumbKey, pixmap);
                    video->changed();
                })
                .onFailed([](auto msg) { qDebug() << msg; });

    const bool thumbsOnly = line.width() <= thumbWidth + 60;
    const bool isHovered = index.data(HoveredItemRole).toBool();

    // play icon overlayed on the thumb
    bool needPlayIcon = isActive;
    if (thumbsOnly) needPlayIcon = needPlayIcon && !isHovered;
    if (needPlayIcon) painter->drawPixmap(0, 0, playIcon);

    if (!thumbsOnly) {
        // text color
        if (isSelected)
            painter->setPen(QPen(option.palette.highlightedText(), 0));
        else
            painter->setPen(QPen(option.palette.text(), 0));

        // title
        QStringRef title(&video->getTitle());
        QString elidedTitle = video->getTitle();
        static const int titleFlags = Qt::AlignTop | Qt::TextWordWrap;
        QRect textBox = line.adjusted(padding + thumbWidth, padding, -padding, 0);
        textBox = painter->boundingRect(textBox, titleFlags, elidedTitle);
        while (textBox.height() > 55 && elidedTitle.length() > 10) {
#if QT_VERSION < QT_VERSION_CHECK(5, 6, 0)
            title = title.left(title.length() - 1);
#elif QT_VERSION < QT_VERSION_CHECK(5, 8, 0)
            title.truncate(title.length() - 1);
#else
            title.chop(1);
#endif
            elidedTitle = title.trimmed() + QStringLiteral("…");
            textBox = painter->boundingRect(textBox, titleFlags, elidedTitle);
        }
        painter->drawText(textBox, titleFlags, elidedTitle);

        painter->setFont(smallerFont);
        painter->setOpacity(.5);
        QFontMetrics fontMetrics = painter->fontMetrics();
        static const int flags = Qt::AlignLeft | Qt::AlignTop;

        // published date
        const QString &published = video->getFormattedPublished();
        QSize textSize(fontMetrics.size(Qt::TextSingleLine, published));
        QPoint textPoint(padding + thumbWidth, padding * 2 + textBox.height());
        textBox = QRect(textPoint, textSize);
        painter->drawText(textBox, flags, published);

        bool elided = false;

        // author
        if (!listView || listView->isClickableAuthors()) {
            bool authorHovered = isHovered && index.data(AuthorHoveredRole).toBool();

            painter->save();
            painter->setFont(smallerBoldFont);
            if (!isSelected) {
                if (authorHovered)
                    painter->setPen(QPen(option.palette.brush(QPalette::Highlight), 0));
            }
            const QString &author = video->getChannelTitle();
            textPoint.setX(textBox.right() + padding);
            textSize = QSize(painter->fontMetrics().size(Qt::TextSingleLine, author));
            textBox = QRect(textPoint, textSize);
            authorRects.insert(index.row(), textBox);
            if (textBox.right() > line.width() - padding) {
                textBox.setRight(line.width());
                elided = drawElidedText(painter, textBox, flags, author);
            } else {
                painter->drawText(textBox, flags, author);
            }
            painter->restore();
        }

        // view count
        if (video->getViewCount() > 0) {
            const QString &viewCount = video->getFormattedViewCount();
            textPoint.setX(textBox.right() + padding);
            textSize = QSize(fontMetrics.size(Qt::TextSingleLine, viewCount));
            if (elided || textPoint.x() + textSize.width() > line.width() - padding) {
                textPoint.setX(thumbWidth + padding);
                textPoint.setY(textPoint.y() + textSize.height() + padding);
            }
            textBox = QRect(textPoint, textSize);
            if (textBox.bottom() <= line.height()) {
                painter->drawText(textBox, flags, viewCount);
            }
        }

        if (downloadInfo) {
            const QString &def = VideoDefinition::forCode(video->getDefinitionCode()).getName();
            textPoint.setX(textBox.right() + padding);
            textSize = QSize(fontMetrics.size(Qt::TextSingleLine, def));
            textBox = QRect(textPoint, textSize);
            painter->drawText(textBox, flags, def);
        }

    } else {
        // thumbs only
        if (isHovered) {
            painter->setFont(smallerFont);
            painter->setPen(Qt::white);
            QStringRef title(&video->getTitle());
            QString elidedTitle = video->getTitle();
            static const int titleFlags = Qt::AlignTop | Qt::TextWordWrap;
            QRect textBox(padding, padding, thumbWidth - padding * 2, thumbHeight - padding * 2);
            textBox = painter->boundingRect(textBox, titleFlags, elidedTitle);
            while (textBox.height() > 55 && elidedTitle.length() > 10) {
#if QT_VERSION < QT_VERSION_CHECK(5, 6, 0)
                title = title.left(title.length() - 1);
#elif QT_VERSION < QT_VERSION_CHECK(5, 8, 0)
                title.truncate(title.length() - 1);
#else
                title.chop(1);
#endif
                elidedTitle = title.trimmed() + QStringLiteral("…");
                textBox = painter->boundingRect(textBox, titleFlags, elidedTitle);
            }
            painter->fillRect(QRect(0, 0, thumbWidth, textBox.height() + padding * 2),
                              QColor(0, 0, 0, 128));
            painter->drawText(textBox, titleFlags, elidedTitle);
        }
    }

    painter->restore();

    if (downloadInfo) paintDownloadInfo(painter, option, index);
}

void PlaylistItemDelegate::paintActiveOverlay(QPainter *painter,
                                              const QStyleOptionViewItem &option,
                                              const QRect &line) const {
    painter->save();
    painter->setOpacity(.2);
    painter->fillRect(line, option.palette.highlight());
    painter->restore();
}

void PlaylistItemDelegate::drawTime(QPainter *painter,
                                    const QString &time,
                                    const QRect &line) const {
    static const int timePadding = 4;
    QRect textBox = painter->boundingRect(line, Qt::AlignLeft | Qt::AlignTop, time);
    // add padding
    textBox.adjust(0, 0, timePadding, 0);
    // move to bottom right corner of the thumb
    textBox.translate(thumbWidth - textBox.width(), thumbHeight - textBox.height());

    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::black);
    painter->setOpacity(.5);
    painter->drawRect(textBox);
    painter->restore();

    painter->save();
    painter->setPen(Qt::white);
    painter->setFont(smallerFont);
    painter->drawText(textBox, Qt::AlignCenter, time);
    painter->restore();
}

void PlaylistItemDelegate::paintDownloadInfo(QPainter *painter,
                                             const QStyleOptionViewItem &option,
                                             const QModelIndex &index) const {
    // get the video metadata
    const DownloadItemPointer downloadItemPointer =
            index.data(DownloadItemRole).value<DownloadItemPointer>();
    const DownloadItem *downloadItem = downloadItemPointer.data();

    painter->save();

    const QRect line(0, 0, option.rect.width() / 2, option.rect.height());

    painter->translate(option.rect.topLeft());
    painter->translate(line.width(), 0);

    QString message;
    DownloadItemStatus status = downloadItem->status();

    if (status == Downloading) {
        QString downloaded = DownloadItem::formattedFilesize(downloadItem->bytesReceived());
        QString total = DownloadItem::formattedFilesize(downloadItem->bytesTotal());
        QString speed = DownloadItem::formattedSpeed(downloadItem->currentSpeed());
        QString eta = DownloadItem::formattedTime(downloadItem->remainingTime());

        message = tr("%1 of %2 (%3) — %4").arg(downloaded, total, speed, eta);
    } else if (status == Starting) {
        message = tr("Preparing");
    } else if (status == Failed) {
        message = tr("Failed") + " — " + downloadItem->errorMessage();
    } else if (status == Finished) {
        message = tr("Completed");
    } else if (status == Idle) {
        message = tr("Stopped");
    }

    // progressBar->setPalette(option.palette);
    if (status == Finished) {
        progressBar->setValue(100);
        progressBar->setEnabled(true);
    } else if (status == Downloading) {
        progressBar->setValue(downloadItem->currentPercent());
        progressBar->setEnabled(true);
    } else {
        progressBar->setValue(0);
        progressBar->setEnabled(false);
    }

    int progressBarWidth = line.width() - padding * 4 - 16;
    progressBar->setMaximumWidth(progressBarWidth);
    progressBar->setMinimumWidth(progressBarWidth);
    painter->save();
    painter->translate(padding, padding);
    progressBar->render(painter);
    painter->restore();

    bool downloadButtonHovered = false;
    bool downloadButtonPressed = false;
    const bool isHovered = index.data(HoveredItemRole).toBool();
    if (isHovered) {
        downloadButtonHovered = index.data(DownloadButtonHoveredRole).toBool();
        downloadButtonPressed = index.data(DownloadButtonPressedRole).toBool();
    }
    QIcon::Mode iconMode;
    if (downloadButtonPressed)
        iconMode = QIcon::Selected;
    else if (downloadButtonHovered)
        iconMode = QIcon::Active;
    else
        iconMode = QIcon::Normal;

    if (status != Finished && status != Failed && status != Idle) {
        if (downloadButtonHovered) message = tr("Stop downloading");
        painter->save();
        QIcon closeIcon = IconUtils::icon("window-close");
        painter->drawPixmap(downloadButtonRect(line), closeIcon.pixmap(16, 16, iconMode));
        painter->restore();
    }

    else if (status == Finished) {
        if (downloadButtonHovered)
#ifdef APP_MAC
            message = tr("Show in %1").arg("Finder");
#else
            message = tr("Open parent folder");
#endif
        painter->save();
        QIcon searchIcon = IconUtils::icon("system-search");
        painter->drawPixmap(downloadButtonRect(line), searchIcon.pixmap(16, 16, iconMode));
        painter->restore();
    }

    else if (status == Failed || status == Idle) {
        if (downloadButtonHovered) message = tr("Restart downloading");
        painter->save();
        QIcon searchIcon = IconUtils::icon("view-refresh");
        painter->drawPixmap(downloadButtonRect(line), searchIcon.pixmap(16, 16, iconMode));
        painter->restore();
    }

    QRect textBox = line.adjusted(padding, padding * 2 + progressBar->sizeHint().height(),
                                  -2 * padding, -padding);
    textBox = painter->boundingRect(textBox, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
                                    message);
    painter->drawText(textBox, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, message);

    painter->restore();
}

QRect PlaylistItemDelegate::downloadButtonRect(const QRect &line) const {
    return QRect(line.width() - padding * 2 - 16,
                 padding + progressBar->sizeHint().height() / 2 - 8, 16, 16);
}

QRect PlaylistItemDelegate::authorRect(const QModelIndex &index) const {
    return authorRects.value(index.row());
}
