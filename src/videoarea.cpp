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

#include "videoarea.h"
#include "loadingwidget.h"
#include "mainwindow.h"
#include "playlistmodel.h"
#include "video.h"
#include "videomimedata.h"
#ifdef Q_OS_MAC
#include "macutils.h"
#endif
#include "fontutils.h"
#include "snapshotpreview.h"

namespace {

class MessageWidget : public QWidget {
public:
    MessageWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setAutoFillBackground(true);

        QBoxLayout *l = new QHBoxLayout(this);
        l->setMargin(32);
        l->setSpacing(32);
        l->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        QLabel *arrowLabel = new QLabel("←");
        arrowLabel->setFont(FontUtils::light(64));
        l->addWidget(arrowLabel);

        QLabel *msgLabel = new QLabel(tr("Pick a video"));
        msgLabel->setFont(FontUtils::light(32));
        l->addWidget(msgLabel);
    }
};
} // namespace

VideoArea::VideoArea(QWidget *parent)
    : QWidget(parent), videoWidget(nullptr), messageWidget(nullptr) {
    setAttribute(Qt::WA_OpaquePaintEvent);

    QBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);

    stackedLayout = new QStackedLayout();
    layout->addLayout(stackedLayout);

#ifdef APP_SNAPSHOT
    snapshotPreview = new SnapshotPreview();
    connect(stackedLayout, SIGNAL(currentChanged(int)), snapshotPreview, SLOT(hide()));
#endif

    setAcceptDrops(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
            SLOT(showContextMenu(const QPoint &)));
}

void VideoArea::setVideoWidget(QWidget *videoWidget) {
    this->videoWidget = videoWidget;
    stackedLayout->addWidget(videoWidget);
}

void VideoArea::setLoadingWidget(LoadingWidget *loadingWidget) {
    this->loadingWidget = loadingWidget;
    stackedLayout->addWidget(loadingWidget);
    stackedLayout->setCurrentWidget(loadingWidget);
}

void VideoArea::showVideo() {
    if (videoWidget) stackedLayout->setCurrentWidget(videoWidget);
    loadingWidget->clear();
}

void VideoArea::showPickMessage() {
    if (!messageWidget) {
        messageWidget = new MessageWidget();
        stackedLayout->addWidget(messageWidget);
    }
    stackedLayout->setCurrentWidget(messageWidget);
}

void VideoArea::showLoading(Video *video) {
    loadingWidget->setVideo(video);
    stackedLayout->setCurrentWidget(loadingWidget);
}

#ifdef APP_SNAPSHOT
void VideoArea::showSnapshotPreview(const QPixmap &pixmap) {
    bool soundOnly = false;
#ifdef APP_MAC
    soundOnly = MainWindow::instance()->isReallyFullScreen();
#endif
    snapshotPreview->start(videoWidget, pixmap, soundOnly);
}

void VideoArea::hideSnapshotPreview() {}
#endif

void VideoArea::clear() {
    loadingWidget->clear();
    stackedLayout->setCurrentWidget(loadingWidget);
}

void VideoArea::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) emit doubleClicked();
}

void VideoArea::dragEnterEvent(QDragEnterEvent *event) {
    // qDebug() << event->mimeData()->formats();
    if (event->mimeData()->hasFormat("application/x-minitube-video")) {
        event->acceptProposedAction();
    }
}

void VideoArea::dropEvent(QDropEvent *event) {
    const VideoMimeData *videoMimeData = qobject_cast<const VideoMimeData *>(event->mimeData());
    if (!videoMimeData) return;

    QVector<Video *> droppedVideos = videoMimeData->getVideos();
    if (droppedVideos.isEmpty()) return;
    Video *video = droppedVideos.at(0);
    int row = listModel->rowForVideo(video);
    if (row != -1) listModel->setActiveRow(row);
    event->acceptProposedAction();
}

void VideoArea::showContextMenu(const QPoint &point) {
    QMenu *menu = MainWindow::instance()->getMenu("video");
    menu->exec(mapToGlobal(point));
}
