#ifndef PLAYLISTWIDGET_H
#define PLAYLISTWIDGET_H

#include <QtGui>
#include "thlibrary/thblackbar.h"

class PlaylistWidget : public QWidget
{
public:
    PlaylistWidget(QWidget *parent, THBlackBar *tabBar, QListView *listView);
};

#endif // PLAYLISTWIDGET_H
