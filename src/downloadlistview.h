#ifndef DOWNLOADLISTVIEW_H
#define DOWNLOADLISTVIEW_H

#include <QtGui>

class DownloadListView : public QListView {

    Q_OBJECT

public:
    DownloadListView(QWidget *parent = 0);

protected:
    void leaveEvent(QEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    bool isHoveringPlayIcon(QMouseEvent *event);

signals:
    void downloadButtonPushed(QModelIndex index);

};

#endif // DOWNLOADLISTVIEW_H
