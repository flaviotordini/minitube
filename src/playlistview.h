#ifndef PLAYLISTVIEW_H
#define PLAYLISTVIEW_H

#include <QtGui>

class PlaylistView : public QListView {

    Q_OBJECT

public:
    PlaylistView(QWidget *parent = 0);
    void setClickableAuthors(bool enabled) { clickableAuthors = enabled; }

protected:
    void leaveEvent(QEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

signals:
    void authorPushed(QModelIndex index);

private slots:
    void itemEntered(const QModelIndex &index);

private:
    bool isHoveringAuthor(QMouseEvent *event);
    bool isShowMoreItem(const QModelIndex &index);
    bool isHoveringThumbnail(QMouseEvent *event);

    bool clickableAuthors;

};

#endif // PLAYLISTVIEW_H
