#include "playlistview.h"
#include "ListModel.h"
#include "playlist/PrettyItemDelegate.h"

PlaylistView::PlaylistView(QWidget *parent) : QListView(parent) {
    connect(this, SIGNAL(entered(const QModelIndex &)), SLOT(itemEntered(const QModelIndex &)));
    setMouseTracking(true);
}

void PlaylistView::itemEntered(const QModelIndex &index) {
    ListModel *listModel = dynamic_cast<ListModel *>(model());
    if (listModel) listModel->setHoveredRow(index.row());
}

void PlaylistView::leaveEvent(QEvent * /* event */) {
    ListModel *listModel = dynamic_cast<ListModel *>(model());
    if (listModel) listModel->clearHover();
}

void PlaylistView::mouseMoveEvent(QMouseEvent *event) {
    // qDebug() << "PlaylistView::mouseMoveEvent" << event->pos();

    QListView::mouseMoveEvent(event);

    if (isHoveringAuthor(event)) {

        // check for special "message" item
        ListModel *listModel = dynamic_cast<ListModel *>(model());
        if (listModel && listModel->rowCount() == indexAt(event->pos()).row())
            return;

        QMetaObject::invokeMethod(model(), "enterAuthorHover");
        setCursor(Qt::PointingHandCursor);
    } else {
        QMetaObject::invokeMethod(model(), "exitAuthorHover");
        unsetCursor();
    }

}

void PlaylistView::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton
        && isHoveringAuthor(event)) {
        QMetaObject::invokeMethod(model(), "enterAuthorPressed");
        event->ignore();
    } else {
        QListView::mousePressEvent(event);
    }
}

void PlaylistView::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        QMetaObject::invokeMethod(model(), "exitAuthorPressed");
        if (isHoveringAuthor(event))
            emit authorPushed(indexAt(event->pos()));
    } else {
        QListView::mousePressEvent(event);
    }
}

bool PlaylistView::isHoveringAuthor(QMouseEvent *event) {
    const QModelIndex itemIndex = indexAt(event->pos());
    const QRect itemRect = visualRect(itemIndex);
    // qDebug() << " itemRect.x()" <<  itemRect.x();

    PrettyItemDelegate *delegate = dynamic_cast<PrettyItemDelegate *>(itemDelegate());
    if (!delegate) return false;

    QRect rect = delegate->authorRect(itemIndex);

    const int x = event->x() - itemRect.x() - rect.x();
    const int y = event->y() - itemRect.y() - rect.y();
    bool ret = x > 0 && x < rect.width() && y > 0 && y < rect.height();

    return ret;
}
