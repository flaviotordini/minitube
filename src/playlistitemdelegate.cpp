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
#include "playlistmodel.h"
#include "playlistview.h"
#include "fontutils.h"
#include "downloaditem.h"
#include "iconutils.h"
#include "videodefinition.h"
#include "video.h"
#include "datautils.h"

const int PlaylistItemDelegate::THUMB_HEIGHT = 90;
const int PlaylistItemDelegate::THUMB_WIDTH = 160;
const int PlaylistItemDelegate::PADDING = 10;

namespace {

void drawElidedText(QPainter *painter, const QRect &textBox, const int flags, const QString &text) {
    QString elidedText = QFontMetrics(painter->font()).elidedText(text, Qt::ElideRight, textBox.width(), flags);
    painter->drawText(textBox, 0, elidedText);
}

}

PlaylistItemDelegate::PlaylistItemDelegate(QObject* parent, bool downloadInfo)
    : QStyledItemDelegate(parent),
      downloadInfo(downloadInfo),
      progressBar(0) {

    listView = qobject_cast<PlaylistView*>(parent);

    boldFont.setBold(true);
    smallerBoldFont = FontUtils::smallBold();
    smallerFont = FontUtils::small();

    if (downloadInfo) {
        progressBar = new QProgressBar(qApp->activeWindow());
        QPalette palette = progressBar->palette();
        palette.setColor(QPalette::Window, Qt::transparent);
        progressBar->setPalette(palette);
        progressBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        progressBar->hide();
    } else createPlayIcon();
}

PlaylistItemDelegate::~PlaylistItemDelegate() {
    if (progressBar) delete progressBar;
}

void PlaylistItemDelegate::createPlayIcon() {
    qreal maxRatio = IconUtils::maxSupportedPixelRatio();
    playIcon = QPixmap(THUMB_WIDTH * maxRatio, THUMB_HEIGHT * maxRatio);
    playIcon.setDevicePixelRatio(maxRatio);
    playIcon.fill(Qt::transparent);

    QPixmap tempPixmap(THUMB_WIDTH * maxRatio, THUMB_HEIGHT * maxRatio);
    tempPixmap.setDevicePixelRatio(maxRatio);
    tempPixmap.fill(Qt::transparent);
    QPainter painter(&tempPixmap);
    painter.setRenderHints(QPainter::Antialiasing, true);

    const int hPadding = PADDING*6;
    const int vPadding = PADDING*2;

    QPolygon polygon;
    polygon << QPoint(hPadding, vPadding)
            << QPoint(THUMB_WIDTH-hPadding, THUMB_HEIGHT/2)
            << QPoint(hPadding, THUMB_HEIGHT-vPadding);
    painter.setBrush(Qt::white);
    QPen pen;
    pen.setColor(Qt::white);
    pen.setWidth(PADDING);
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setCapStyle(Qt::RoundCap);
    painter.setPen(pen);
    painter.drawPolygon(polygon);
    painter.end();

    QPainter painter2(&playIcon);
    painter2.setOpacity(.75);
    painter2.drawPixmap(0, 0, tempPixmap);
}

QSize PlaylistItemDelegate::sizeHint( const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/ ) const {
    return QSize(THUMB_WIDTH, THUMB_HEIGHT + 1);
}

void PlaylistItemDelegate::paint( QPainter* painter,
                                  const QStyleOptionViewItem& option, const QModelIndex& index ) const {

    int itemType = index.data(ItemTypeRole).toInt();
    if (itemType == ItemTypeVideo) {
        QStyleOptionViewItem opt = QStyleOptionViewItem(option);
        initStyleOption(&opt, index);
        opt.text = "";
        opt.widget->style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);
        paintBody(painter, opt, index);
    } else
        QStyledItemDelegate::paint( painter, option, index );

}

void PlaylistItemDelegate::paintBody( QPainter* painter,
                                      const QStyleOptionViewItem& option,
                                      const QModelIndex& index ) const {
    painter->save();
    painter->translate( option.rect.topLeft() );

    QRect line(0, 0, option.rect.width(), option.rect.height());
    if (downloadInfo) line.setWidth(line.width() / 2);

    const bool isActive = index.data( ActiveTrackRole ).toBool();
    const bool isSelected = option.state & QStyle::State_Selected;

    // get the video metadata
    const Video *video = index.data(VideoRole).value<VideoPointer>().data();

    // draw the "current track" highlight underneath the text
    if (isActive && !isSelected)
        paintActiveOverlay(painter, option, line);

    // separator
    painter->setPen(option.palette.color(QPalette::Midlight));
    painter->drawLine(THUMB_WIDTH, THUMB_HEIGHT, option.rect.width(), THUMB_HEIGHT);
    if (!video->thumbnail().isNull())
        painter->setPen(Qt::black);
    painter->drawLine(0, THUMB_HEIGHT, THUMB_WIDTH-1, THUMB_HEIGHT);

    // thumb
    painter->drawPixmap(0, 0, video->thumbnail());

    const bool thumbsOnly = line.width() <= THUMB_WIDTH + 60;
    const bool isHovered = index.data(HoveredItemRole).toBool();

    // play icon overlayed on the thumb
    if (isActive && (!isHovered && thumbsOnly))
        painter->drawPixmap(0, 0, playIcon);

    // time
    if (video->duration() > 0)
        drawTime(painter, video->formattedDuration(), line);

    if (!thumbsOnly) {

        // text color
        if (isSelected)
            painter->setPen(QPen(option.palette.highlightedText(), 0));
        else
            painter->setPen(QPen(option.palette.text(), 0));

        // title
        QString videoTitle = video->title();
        QString v = videoTitle;
        const int flags = Qt::AlignTop | Qt::TextWordWrap;
        QRect textBox = line.adjusted(PADDING+THUMB_WIDTH, PADDING, 0, 0);
        textBox = painter->boundingRect(textBox, flags, v);
        while (textBox.height() > 55 && v.length() > 10) {
            videoTitle.chop(1);
            v = videoTitle.trimmed().append(QStringLiteral("…"));
            textBox = painter->boundingRect(textBox, flags, v);
        }
        painter->drawText(textBox, flags, v);

        painter->setFont(smallerFont);
        painter->setOpacity(.5);

        // published date
        QString publishedString = DataUtils::formatDateTime(video->published());
        QSize stringSize(QFontMetrics(painter->font()).size( Qt::TextSingleLine, publishedString ) );
        QPoint textLoc(PADDING+THUMB_WIDTH, PADDING*2 + textBox.height());
        QRect publishedTextBox(textLoc , stringSize);
        painter->drawText(publishedTextBox, Qt::AlignLeft | Qt::AlignTop, publishedString);

        // author
        if (!listView || listView->isClickableAuthors()) {
            bool authorHovered = isHovered && index.data(AuthorHoveredRole).toBool();

            painter->save();
            painter->setFont(smallerBoldFont);
            if (!isSelected) {
                if (authorHovered)
                    painter->setPen(QPen(option.palette.brush(QPalette::Highlight), 0));
            }
            const QString &authorString = video->channelTitle();
            textLoc.setX(textLoc.x() + stringSize.width() + PADDING);
            stringSize = QSize(QFontMetrics(painter->font()).size( Qt::TextSingleLine, authorString ) );
            QRect authorTextBox(textLoc , stringSize);
            authorRects.insert(index.row(), authorTextBox);
            if (authorTextBox.right() > line.width()) authorTextBox.setRight(line.width());
            drawElidedText(painter, authorTextBox, Qt::AlignLeft | Qt::AlignTop, authorString);
            painter->restore();
        }

        // view count
        if (video->viewCount() > 0) {
            QLocale locale;
            QString viewCountString = tr("%1 views").arg(locale.toString(video->viewCount()));
            textLoc.setX(textLoc.x() + stringSize.width() + PADDING);
            stringSize = QSize(QFontMetrics(painter->font()).size( Qt::TextSingleLine, viewCountString ) );
            QRect viewCountTextBox(textLoc , stringSize);
            if (viewCountTextBox.right() > line.width()) viewCountTextBox.setRight(line.width());
            drawElidedText(painter, viewCountTextBox, Qt::AlignLeft | Qt::AlignBottom, viewCountString);
        }

        if (downloadInfo) {
            const QString definitionString = VideoDefinition::getDefinitionFor(video->getDefinitionCode()).getName();
            textLoc.setX(textLoc.x() + stringSize.width() + PADDING);
            stringSize = QSize(QFontMetrics(painter->font()).size( Qt::TextSingleLine, definitionString ) );
            QRect viewCountTextBox(textLoc , stringSize);
            painter->drawText(viewCountTextBox, Qt::AlignLeft | Qt::AlignBottom, definitionString);
        }

    } else {

        if (isHovered) {
            painter->setFont(smallerFont);
            painter->setPen(Qt::white);
            QString videoTitle = video->title();
            QString v = videoTitle;
            const int flags = Qt::AlignTop | Qt::TextWordWrap;
            QRect textBox(PADDING, PADDING, THUMB_WIDTH - PADDING*2, THUMB_HEIGHT - PADDING*2);
            textBox = painter->boundingRect(textBox, flags, v);
            while (textBox.height() > THUMB_HEIGHT && v.length() > 10) {
                videoTitle.chop(1);
                v = videoTitle.trimmed().append(QStringLiteral("…"));
                textBox = painter->boundingRect(textBox, flags, v);
            }
            painter->fillRect(QRect(0, 0, THUMB_WIDTH, textBox.height() + PADDING*2), QColor(0, 0, 0, 128));
            painter->drawText(textBox, flags, v);
        }

    }

    painter->restore();

    if (downloadInfo) paintDownloadInfo(painter, option, index);

}

void PlaylistItemDelegate::paintActiveOverlay(QPainter *painter, const QStyleOptionViewItem& option, const QRect &line) const {
    painter->save();
    painter->setOpacity(.2);
    painter->fillRect(line, option.palette.highlight());
    painter->restore();
}

void PlaylistItemDelegate::drawTime(QPainter *painter, const QString &time, const QRect &line) const {
    static const int timePadding = 4;
    QRect textBox = painter->boundingRect(line, Qt::AlignLeft | Qt::AlignTop, time);
    // add padding
    textBox.adjust(0, 0, timePadding, 0);
    // move to bottom right corner of the thumb
    textBox.translate(THUMB_WIDTH - textBox.width(), THUMB_HEIGHT - textBox.height());

    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::black);
    painter->setOpacity(.5);
    painter->drawRect(textBox);
    painter->restore();

    painter->save();
    painter->setPen(Qt::white);
    painter->drawText(textBox, Qt::AlignCenter, time);
    painter->restore();
}

void PlaylistItemDelegate::paintDownloadInfo( QPainter* painter,
                                              const QStyleOptionViewItem& option,
                                              const QModelIndex& index ) const {

    // get the video metadata
    const DownloadItemPointer downloadItemPointer = index.data(DownloadItemRole).value<DownloadItemPointer>();
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

        message = tr("%1 of %2 (%3) — %4").arg(
                    downloaded,
                    total,
                    speed,
                    eta
                    );
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

    int progressBarWidth = line.width() - PADDING*4 - 16;
    progressBar->setMaximumWidth(progressBarWidth);
    progressBar->setMinimumWidth(progressBarWidth);
    painter->save();
    painter->translate(PADDING, PADDING);
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
    if (downloadButtonPressed) iconMode = QIcon::Selected;
    else if (downloadButtonHovered) iconMode = QIcon::Active;
    else iconMode = QIcon::Normal;

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

    QRect textBox = line.adjusted(PADDING, PADDING*2 + progressBar->sizeHint().height(), -2 * PADDING, -PADDING);
    textBox = painter->boundingRect( textBox, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, message);
    painter->drawText(textBox, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, message);

    painter->restore();
}

QRect PlaylistItemDelegate::downloadButtonRect(const QRect &line) const {
    return QRect(
                line.width() - PADDING*2 - 16,
                PADDING + progressBar->sizeHint().height() / 2 - 8,
                16,
                16);
}

QRect PlaylistItemDelegate::authorRect(const QModelIndex& index) const {
    return authorRects.value(index.row());
}
