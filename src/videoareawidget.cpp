#include "videoareawidget.h"
#include "videomimedata.h"

VideoAreaWidget::VideoAreaWidget(QWidget *parent) : QWidget(parent) {
    QBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setMargin(0);
    vLayout->setSpacing(0);
    
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
    
    setLayout(vLayout);
    setAcceptDrops(true);
    
}

void VideoAreaWidget::setVideoWidget(QWidget *videoWidget) {
    this->videoWidget = videoWidget;
    videoWidget->setMouseTracking(true);
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
    // loadingWidget->setError(message);
    messageLabel->setText(message);
    messageLabel->show();
    stackedLayout->setCurrentWidget(loadingWidget);
}

void VideoAreaWidget::showLoading(Video *video) {
    this->loadingWidget->setVideo(video);
    stackedLayout->setCurrentWidget(loadingWidget);
    messageLabel->hide();
    messageLabel->clear();
}

void VideoAreaWidget::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton)
        emit doubleClicked();
}

void VideoAreaWidget::mousePressEvent(QMouseEvent *event) {
    switch(event->button() == Qt::RightButton)
            emit rightClicked();
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
