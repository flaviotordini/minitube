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

#include "playlistview.h"
#include "painterutils.h"
#include "playlistitemdelegate.h"
#include "playlistmodel.h"

PlaylistView::PlaylistView(QWidget *parent) : QListView(parent), clickableAuthors(true) {
    setItemDelegate(new PlaylistItemDelegate(this));
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);

    // dragndrop
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::DragDrop);

    // cosmetics
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setFrameShape(QFrame::NoFrame);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setUniformItemSizes(true);

    connect(this, SIGNAL(entered(const QModelIndex &)), SLOT(itemEntered(const QModelIndex &)));
    setMouseTracking(true);

    QScrollBar *vScrollbar = verticalScrollBar();
    connect(vScrollbar, &QAbstractSlider::valueChanged, this, [this, vScrollbar](int value) {
        if (isVisible() && value == vScrollbar->maximum()) {
            PlaylistModel *listModel = qobject_cast<PlaylistModel *>(model());
            listModel->searchMore();
        }
    });
    setMinimumHeight(PlaylistItemDelegate::thumbHeight * 4);

    setMinimumWidth(PlaylistItemDelegate::thumbWidth);
#ifndef APP_MAC
    setMinimumWidth(minimumWidth() + vScrollbar->width());
#endif
}

void PlaylistView::itemEntered(const QModelIndex &index) {
    PlaylistModel *listModel = qobject_cast<PlaylistModel *>(model());
    if (listModel) listModel->setHoveredRow(index.row());
}

void PlaylistView::leaveEvent(QEvent *event) {
    QListView::leaveEvent(event);
    PlaylistModel *listModel = qobject_cast<PlaylistModel *>(model());
    if (listModel) listModel->clearHover();
}

void PlaylistView::mouseMoveEvent(QMouseEvent *event) {
    if (isHoveringThumbnail(event)) {
        setCursor(Qt::PointingHandCursor);
    } else if (isShowMoreItem(indexAt(event->pos()))) {
        setCursor(Qt::PointingHandCursor);
    } else if (isHoveringAuthor(event)) {
        QMetaObject::invokeMethod(model(), "enterAuthorHover");
        setCursor(Qt::PointingHandCursor);
    } else {
        QMetaObject::invokeMethod(model(), "exitAuthorHover");
        unsetCursor();
    }
    QListView::mouseMoveEvent(event);
}

void PlaylistView::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if (isHoveringAuthor(event)) {
            QMetaObject::invokeMethod(model(), "enterAuthorPressed");
        } else if (isHoveringThumbnail(event)) {
            const QModelIndex index = indexAt(event->pos());
            emit activated(index);
            unsetCursor();
            return;
        }
        QListView::mousePressEvent(event);
    }
}

void PlaylistView::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        QMetaObject::invokeMethod(model(), "exitAuthorPressed");
        const QModelIndex index = indexAt(event->pos());
        if (isHoveringAuthor(event)) {
            emit authorPushed(index);
        } else if (isShowMoreItem(index)) {
            PlaylistModel *listModel = qobject_cast<PlaylistModel *>(model());
            listModel->searchMore();
            unsetCursor();
        }
    }
    QListView::mouseReleaseEvent(event);
}

bool PlaylistView::isHoveringAuthor(QMouseEvent *event) {
    if (!clickableAuthors) return false;

    const QModelIndex itemIndex = indexAt(event->pos());
    const QRect itemRect = visualRect(itemIndex);
    // qDebug() << " itemRect.x()" <<  itemRect.x();

    PlaylistItemDelegate *delegate = qobject_cast<PlaylistItemDelegate *>(itemDelegate());
    if (!delegate) return false;

    QRect rect = delegate->authorRect(itemIndex);

    const int x = event->x() - itemRect.x() - rect.x();
    const int y = event->y() - itemRect.y() - rect.y();
    bool ret = x > 0 && x < rect.width() && y > 0 && y < rect.height();

    return ret;
}

bool PlaylistView::isHoveringThumbnail(QMouseEvent *event) {
    const QModelIndex index = indexAt(event->pos());
    const QRect itemRect = visualRect(index);
    static const QRect thumbRect(0, 0, PlaylistItemDelegate::thumbWidth,
                                 PlaylistItemDelegate::thumbHeight);
    const int x = event->x() - itemRect.x() - thumbRect.x();
    const int y = event->y() - itemRect.y() - thumbRect.y();
    return x > 0 && x < thumbRect.width() && y > 0 && y < thumbRect.height();
}

bool PlaylistView::isShowMoreItem(const QModelIndex &index) {
    return model()->rowCount() > 1 && model()->rowCount() == index.row() + 1;
}
