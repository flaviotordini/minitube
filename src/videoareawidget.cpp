#include "videoareawidget.h"
#include "videomimedata.h"

VideoAreaWidget::VideoAreaWidget(QWidget *parent) : QWidget(parent) {
    stackedLayout = new QStackedLayout(this);
    setLayout(stackedLayout);
    setAcceptDrops(true);

    // mouse autohide
    setMouseTracking(true);
    mouseTimer = new QTimer(this);
    mouseTimer->setInterval(5000);
    mouseTimer->setSingleShot(true);
    connect(mouseTimer, SIGNAL(timeout()), SLOT(hideMouse()));
}

void VideoAreaWidget::setVideoWidget(QWidget *videoWidget) {
    this->videoWidget = videoWidget;
    stackedLayout->addWidget(videoWidget);
}

void VideoAreaWidget::setLoadingWidget(LoadingWidget *loadingWidget) {
    this->loadingWidget = loadingWidget;
    stackedLayout->addWidget(loadingWidget);
}

void VideoAreaWidget::showVideo() {
    stackedLayout->setCurrentWidget(videoWidget);
}

void VideoAreaWidget::showError(QString message) {
    loadingWidget->setError(message);
    stackedLayout->setCurrentWidget(loadingWidget);
}

void VideoAreaWidget::showLoading(Video *video) {
    this->loadingWidget->setVideo(video);
    stackedLayout->setCurrentWidget(loadingWidget);
}

void VideoAreaWidget::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton)
        emit doubleClicked();
}

void VideoAreaWidget::mousePressEvent(QMouseEvent *event) {
    switch(event->button() == Qt::RightButton)
            emit rightClicked();
}

void VideoAreaWidget::mouseMoveEvent(QMouseEvent * /* event */) {
    // show the normal cursor
    unsetCursor();
    // then hide it again after a few seconds
    mouseTimer->start();
}

void VideoAreaWidget::hideMouse() {
    setCursor(QCursor(QBitmap(1,1)));
}

void VideoAreaWidget::dragEnterEvent(QDragEnterEvent *event) {
    qDebug() << event->mimeData()->formats();
    if (event->mimeData()->hasFormat("application/x-minitube-video")) {
        event->acceptProposedAction();
    }
}

void VideoAreaWidget::dropEvent(QDropEvent *event) {

    const VideoMimeData* videoMimeData = dynamic_cast<const VideoMimeData*>( event->mimeData() );
    if(!videoMimeData ) return;

    QList<Video*> droppedVideos = videoMimeData->videos();
    foreach( Video *video, droppedVideos) {
        int row = listModel->rowForVideo(video);
        if (row != -1)
            listModel->setActiveRow(row);
    }
    event->acceptProposedAction();
}
