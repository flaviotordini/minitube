#include "videoareawidget.h"
#include "videomimedata.h"

VideoAreaWidget::VideoAreaWidget(QWidget *parent) : QWidget(parent) {
    QBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setMargin(0);
    vLayout->setSpacing(0);

#ifdef APP_WIN
    QPalette p = palette();
    p.setBrush(QPalette::Window, Qt::black);
    setPalette(p);
    setAutoFillBackground(true);
    setStyleSheet("background:black");
#endif

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
    
    setMouseTracking(true);
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

void VideoAreaWidget::clear() {
    stackedLayout->setCurrentWidget(loadingWidget);
    loadingWidget->clear();
    messageLabel->hide();
    messageLabel->clear();
}

void VideoAreaWidget::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton)
        emit doubleClicked();
}

void VideoAreaWidget::mousePressEvent(QMouseEvent *event) {
    QWidget::mousePressEvent(event);

    if(event->button() == Qt::RightButton)
        emit rightClicked();
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

void VideoAreaWidget::mouseMoveEvent(QMouseEvent *event) {
    QWidget::mouseMoveEvent(event);

#ifdef Q_WS_X11
    QWidget* mainWindow = window();
    if (!mainWindow->isFullScreen()) return;

    // qDebug() << "VideoAreaWidget::mouseMoveEvent" << event->pos();

    const int x = event->pos().x();
    const int y = event->pos().y();

    bool visible = y <= 10;
    bool ret = QMetaObject::invokeMethod(mainWindow, "showFullscreenToolbar", Qt::DirectConnection, Q_ARG(bool, visible));
    if (!ret) qDebug() << "showFullscreenToolbar invokeMethod failed";

    visible = x <= 10;
    ret = QMetaObject::invokeMethod(mainWindow, "showFullscreenPlaylist", Qt::DirectConnection, Q_ARG(bool, visible));
    if (!ret) qDebug() << "showFullscreenPlaylist invokeMethod failed";
#endif
}

void VideoAreaWidget::leaveEvent(QMouseEvent *event) {
    QWidget::leaveEvent(event);

#ifdef Q_WS_X11
    QWidget* mainWindow = window();
    if (!mainWindow->isFullScreen()) return;

    bool visible = false;
    bool ret = QMetaObject::invokeMethod(mainWindow, "showFullscreenToolbar", Qt::DirectConnection, Q_ARG(bool, visible));
    if (!ret) qDebug() << "showFullscreenToolbar invokeMethod failed";

    ret = QMetaObject::invokeMethod(mainWindow, "showFullscreenPlaylist", Qt::DirectConnection, Q_ARG(bool, visible));
    if (!ret) qDebug() << "showFullscreenPlaylist invokeMethod failed";
#endif
}
