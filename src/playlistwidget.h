#ifndef PLAYLISTWIDGET_H
#define PLAYLISTWIDGET_H

#include <QtGui>
#include "segmentedcontrol.h"

class PlaylistWidget : public QWidget
{
public:
    PlaylistWidget(QWidget *parent, SegmentedControl *tabBar, QListView *listView);
};

#endif // PLAYLISTWIDGET_H
