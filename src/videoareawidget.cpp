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

#include "videoareawidget.h"
#include "videomimedata.h"
#include "mainwindow.h"
#ifdef APP_EXTRA
#include "extra.h"
#endif
#ifdef Q_WS_MAC
#include "macutils.h"
#endif

VideoAreaWidget::VideoAreaWidget(QWidget *parent) : QWidget(parent) {
    QBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setMargin(0);
    vLayout->setSpacing(0);

    QPalette p = palette();
    p.setBrush(QPalette::Window, Qt::black);
    setPalette(p);
    setAutoFillBackground(true);

    // hidden message widget
    messageLabel = new QLabel(this);
    messageLabel->setOpenExternalLinks(true);
    messageLabel->setMargin(7);
    messageLabel->setBackgroundRole(QPalette::ToolTipBase);
    messageLabel->setForegroundRole(QPalette::ToolTipText);
    messageLabel->setAutoFillBackground(true);
    messageLabel->setWordWrap(true);
    messageLabel->hide();
    vLayout->addWidget(messageLabel);

    stackedLayout = new QStackedLayout();
    vLayout->addLayout(stackedLayout);

    // snapshotPreview = new QLabel(this);
    // stackedLayout->addWidget(snapshotPreview);
    
    setAcceptDrops(true);
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void VideoAreaWidget::setVideoWidget(QWidget *videoWidget) {
    this->videoWidget = videoWidget;
    videoWidget->setMouseTracking(true);
    stackedLayout->addWidget(videoWidget);
}

void VideoAreaWidget::setLoadingWidget(LoadingWidget *loadingWidget) {
    this->loadingWidget = loadingWidget;
    stackedLayout->addWidget(loadingWidget);
    stackedLayout->setCurrentWidget(loadingWidget);
}

void VideoAreaWidget::showVideo() {
    stackedLayout->setCurrentWidget(videoWidget);
    loadingWidget->clear();
}

void VideoAreaWidget::showError(QString message) {
    // loadingWidget->setError(message);
    messageLabel->setText(message);
    messageLabel->show();
    stackedLayout->setCurrentWidget(loadingWidget);
}

void VideoAreaWidget::showLoading(Video *video) {
    messageLabel->hide();
    messageLabel->clear();
    stackedLayout->setCurrentWidget(loadingWidget);
    loadingWidget->setVideo(video);
}

/*
void VideoAreaWidget::showSnapshotPreview(QPixmap pixmap) {
    snapshotPreview->setPixmap(pixmap);
    stackedLayout->setCurrentWidget(snapshotPreview);
#ifdef APP_EXTRA
    Extra::flashInWidget(snapshotPreview);
#endif
    QTimer::singleShot(1500, this, SLOT(hideSnapshotPreview()));
}

void VideoAreaWidget::hideSnapshotPreview() {
    stackedLayout->setCurrentWidget(videoWidget);
}
*/

void VideoAreaWidget::clear() {
    loadingWidget->clear();
    messageLabel->hide();
    messageLabel->clear();
    // snapshotPreview->clear();
    stackedLayout->setCurrentWidget(loadingWidget);
}

void VideoAreaWidget::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton)
        emit doubleClicked();
}

void VideoAreaWidget::mousePressEvent(QMouseEvent *event) {
    QWidget::mousePressEvent(event);

    if(event->button() == Qt::RightButton)
        emit rightClicked();

    else if (event->button() == Qt::LeftButton) {
        bool isNormalWindow = !window()->isMaximized() &&
                !MainWindow::instance()->isReallyFullScreen();
        if (isNormalWindow) {
            dragPosition = event->globalPos() - window()->frameGeometry().topLeft();
            event->accept();
        }
    }
}

void VideoAreaWidget::mouseMoveEvent(QMouseEvent *event) {
    bool isNormalWindow = !window()->isMaximized() &&
            !MainWindow::instance()->isReallyFullScreen();
    if (event->buttons() & Qt::LeftButton && isNormalWindow) {
        QPoint p = event->globalPos() - dragPosition;
#ifdef Q_WS_MAC
        mac::moveWindowTo(window()->winId(), p.x(), p.y());
#else
        window()->move(p);
#endif
        event->accept();
    }
}

void VideoAreaWidget::dragEnterEvent(QDragEnterEvent *event) {
    // qDebug() << event->mimeData()->formats();
    if (event->mimeData()->hasFormat("application/x-minitube-video")) {
        event->acceptProposedAction();
    }
}

void VideoAreaWidget::dropEvent(QDropEvent *event) {
    
    const VideoMimeData* videoMimeData = dynamic_cast<const VideoMimeData*>( event->mimeData() );
    if(!videoMimeData ) return;
    
    QList<Video*> droppedVideos = videoMimeData->videos();
    if (droppedVideos.isEmpty())
        return;
    Video *video = droppedVideos.first();
    int row = listModel->rowForVideo(video);
    if (row != -1)
        listModel->setActiveRow(row);
    event->acceptProposedAction();
}
