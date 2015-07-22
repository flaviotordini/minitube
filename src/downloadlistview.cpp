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

#include "downloadlistview.h"
#include "downloadmodel.h"
#include "playlistitemdelegate.h"
#include "painterutils.h"

DownloadListView::DownloadListView(QWidget *parent) : QListView(parent) {

}

void DownloadListView::leaveEvent(QEvent * /* event */) {
    DownloadModel *downloadModel = dynamic_cast<DownloadModel *>(model());
    if (downloadModel) downloadModel->clearHover();
}

void DownloadListView::mouseMoveEvent(QMouseEvent *event) {
    // qDebug() << "DownloadListView::mouseMoveEvent" << event->pos();

    QListView::mouseMoveEvent(event);

    if (isHoveringPlayIcon(event)) {
        QMetaObject::invokeMethod(model(), "enterPlayIconHover");
    } else {
        QMetaObject::invokeMethod(model(), "exitPlayIconHover");
    }

}

void DownloadListView::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton
        && isHoveringPlayIcon(event)) {
        QMetaObject::invokeMethod(model(), "enterPlayIconPressed");
    } else {
        QListView::mousePressEvent(event);
    }
}

void DownloadListView::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        QMetaObject::invokeMethod(model(), "exitPlayIconPressed");
        if (isHoveringPlayIcon(event))
            emit downloadButtonPushed(indexAt(event->pos()));
    } else {
        QListView::mousePressEvent(event);
    }
}

bool DownloadListView::isHoveringPlayIcon(QMouseEvent *event) {
    const QModelIndex itemIndex = indexAt(event->pos());
    const QRect itemRect = visualRect(itemIndex);
    // qDebug() << " itemRect.x()" <<  itemRect.x();

    PlaylistItemDelegate *delegate = dynamic_cast<PlaylistItemDelegate *>(itemDelegate());
    if (!delegate) return false;

    QRect buttonRect = delegate->downloadButtonRect(itemRect);

    const int x = event->x() - itemRect.x() - buttonRect.x();
    const int y = event->y() - itemRect.y() - buttonRect.y();
    return x > 0 && x < buttonRect.width() && y > 0 && y < buttonRect.height();
}

void DownloadListView::paintEvent(QPaintEvent *event) {
    QListView::paintEvent(event);
    // PainterUtils::topShadow(viewport());
}
