#include "playlistview.h"
#include "playlistmodel.h"
#include "playlistitemdelegate.h"

PlaylistView::PlaylistView(QWidget *parent) : QListView(parent) {
    clickableAuthors = true;

    setItemDelegate(new PlaylistItemDelegate(this));
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    // dragndrop
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::DragDrop);

    // cosmetics
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setFrameShape(QFrame::NoFrame);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    // setMinimumSize(120, 240);
    setUniformItemSizes(true);

    connect(this, SIGNAL(entered(const QModelIndex &)),
            SLOT(itemEntered(const QModelIndex &)));
    setMouseTracking(true);
}

void PlaylistView::itemEntered(const QModelIndex &index) {
    PlaylistModel *listModel = dynamic_cast<PlaylistModel *>(model());
    if (listModel) listModel->setHoveredRow(index.row());
}

void PlaylistView::leaveEvent(QEvent * /* event */) {
    PlaylistModel *listModel = dynamic_cast<PlaylistModel *>(model());
    if (listModel) listModel->clearHover();
}

void PlaylistView::mouseMoveEvent(QMouseEvent *event) {
    QListView::mouseMoveEvent(event);
    // QWidget::mouseMoveEvent(event);

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
}

void PlaylistView::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if (isHoveringThumbnail(event)) {
            event->accept();
        } else if (isHoveringAuthor(event)) {
            QMetaObject::invokeMethod(model(), "enterAuthorPressed");
            event->ignore();
        } else QListView::mousePressEvent(event);
    } else QListView::mousePressEvent(event);
}

void PlaylistView::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        QMetaObject::invokeMethod(model(), "exitAuthorPressed");
        const QModelIndex index =  indexAt(event->pos());
        if (isHoveringThumbnail(event)) {
            emit activated(index);
            unsetCursor();
        } else if (isHoveringAuthor(event)) {
            emit authorPushed(index);
        } else if (isShowMoreItem(index)) {
            PlaylistModel *listModel = dynamic_cast<PlaylistModel *>(model());
            listModel->searchMore();
            unsetCursor();
        }

    } else {
        QListView::mousePressEvent(event);
    }
}

bool PlaylistView::isHoveringAuthor(QMouseEvent *event) {
    if (!clickableAuthors) return false;

    const QModelIndex itemIndex = indexAt(event->pos());
    const QRect itemRect = visualRect(itemIndex);
    // qDebug() << " itemRect.x()" <<  itemRect.x();

    PlaylistItemDelegate *delegate = dynamic_cast<PlaylistItemDelegate *>(itemDelegate());
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
    static const QRect thumbRect(0, 0, 160, 90);
    const int x = event->x() - itemRect.x() - thumbRect.x();
    const int y = event->y() - itemRect.y() - thumbRect.y();
    return x > 0 && x < thumbRect.width() && y > 0 && y < thumbRect.height();
}

bool PlaylistView::isShowMoreItem(const QModelIndex &index) {
    return model()->rowCount() > 1 &&
            model()->rowCount() == index.row() + 1;
}
