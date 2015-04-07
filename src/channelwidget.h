#ifndef CHANNELWIDGET_H
#define CHANNELWIDGET_H

#include <QtGui>
#include "gridwidget.h"

class VideoSource;
class YTUser;

class ChannelWidget : public GridWidget {

    Q_OBJECT

public:
    ChannelWidget(VideoSource *videoSource, YTUser *user, QWidget *parent = 0);
    
signals:
    void activated(VideoSource *videoSource);

protected:
    void paintEvent(QPaintEvent *event);

private slots:
    void gotUserInfo();
    void gotUserThumbnail(QByteArray bytes);
    void activate();

private:
    QPixmap thumbnail;
    YTUser *user;
    VideoSource *videoSource;

};

#endif // CHANNELWIDGET_H
