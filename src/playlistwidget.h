#ifndef PLAYLISTWIDGET_H
#define PLAYLISTWIDGET_H

#include <QtGui>

class SegmentedControl;

class PlaylistWidget : public QWidget {

    Q_OBJECT

public:
    PlaylistWidget(QWidget *parent, SegmentedControl *tabBar, QListView *listView);

};

#endif // PLAYLISTWIDGET_H
