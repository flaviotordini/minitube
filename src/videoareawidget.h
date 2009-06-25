#ifndef VIDEOAREAWIDGET_H
#define VIDEOAREAWIDGET_H

#include <QWidget>
#include "video.h"
#include "loadingwidget.h"

class VideoAreaWidget : public QWidget {

    Q_OBJECT

public:
    VideoAreaWidget(QWidget *parent);
    void setVideoWidget(QWidget *videoWidget);
    void setLoadingWidget(LoadingWidget *loadingWidget);
    void showLoading(Video* video);
    void showVideo();

signals:
    void doubleClicked();
    void rightClicked();

protected:
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);

private:
    QStackedLayout *stackedLayout;
    QWidget *videoWidget;
    LoadingWidget *loadingWidget;

};

#endif // VIDEOAREAWIDGET_H
