#ifndef PLAYLISTVIEW_H
#define PLAYLISTVIEW_H

#include <QtGui>

class PlaylistView : public QListView {

    Q_OBJECT

public:
    PlaylistView(QWidget *parent = 0);

protected:
    void leaveEvent(QEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    bool isHoveringAuthor(QMouseEvent *event);

signals:
    void authorPushed(QModelIndex index);

private slots:
    void itemEntered(const QModelIndex &index);

};

#endif // PLAYLISTVIEW_H
