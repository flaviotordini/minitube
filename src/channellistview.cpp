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

#include "channellistview.h"
#include "painterutils.h"

ChannelListView::ChannelListView() {
    setSelectionMode(QAbstractItemView::NoSelection);

    // layout
    setSpacing(10);
    setFlow(QListView::LeftToRight);
    setWrapping(true);
    setResizeMode(QListView::Adjust);
    setMovement(QListView::Static);
    setUniformItemSizes(true);

    // cosmetics
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setFrameShape(QFrame::NoFrame);
    setAttribute(Qt::WA_MacShowFocusRect, false);

    verticalScrollBar()->setPageStep(3);
    verticalScrollBar()->setSingleStep(1);

    setMouseTracking(true);
}

void ChannelListView::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton)
        emit contextMenu(event->pos());
    else
        QListView::mousePressEvent(event);
}

void ChannelListView::mouseMoveEvent(QMouseEvent *event) {
    QWidget::mouseMoveEvent(event);
    const QModelIndex index = indexAt(event->pos());
    if (index.isValid())
        setCursor(Qt::PointingHandCursor);
    else
        unsetCursor();
}

void ChannelListView::paintEvent(QPaintEvent *event) {
    if (!errorMessage.isEmpty())
        PainterUtils::centeredMessage(errorMessage, viewport());
    else
        QListView::paintEvent(event);
}
