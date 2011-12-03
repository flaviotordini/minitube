#include "downloadlistview.h"
#include "downloadmodel.h"
#include "playlist/PrettyItemDelegate.h"

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

    PrettyItemDelegate *delegate = dynamic_cast<PrettyItemDelegate *>(itemDelegate());
    if (!delegate) return false;

    QRect buttonRect = delegate->downloadButtonRect(itemRect);

    const int x = event->x() - itemRect.x() - buttonRect.x();
    const int y = event->y() - itemRect.y() - buttonRect.y();
    return x > 0 && x < buttonRect.width() && y > 0 && y < buttonRect.height();
}
