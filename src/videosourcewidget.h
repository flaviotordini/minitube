#ifndef VIDEOSOURCEWIDGET_H
#define VIDEOSOURCEWIDGET_H

#include <QtGui>

class Video;
class VideoSource;

class VideoSourceWidget : public QWidget {

    Q_OBJECT

public:
    VideoSourceWidget(VideoSource *videoSource, QWidget *parent = 0);

signals:
    void activated(VideoSource *videoSource);

protected:
    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

private slots:
    void activate();
    void previewVideo(QList<Video*> videos);
    void setPixmapData(QByteArray bytes);

private:
    QPixmap playPixmap();
    VideoSource *videoSource;
    QPixmap pixmap;
    Video *video;

    bool hovered;
    bool pressed;
};

#endif // VIDEOSOURCEWIDGET_H
