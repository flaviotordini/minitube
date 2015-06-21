/* $BEGIN_LICENSE

This file is part of Minitube.
Copyright 2009, Flavio Tordini <flavio.tordini@gmail.com>

Minitube is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Minitube is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Minitube.  If not, see <http://www.gnu.org/licenses/>.

$END_LICENSE */

#ifndef CHANNELSVIEW_H
#define CHANNELSVIEW_H

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif
#include "view.h"

class ChannelModel;
class YTChannel;

class ChannelView : public QListView, public View {

    Q_OBJECT

public:
    explicit ChannelView(ChannelModel *model, QWidget *parent = 0);

    enum SortBy {
        SortByName = 0,
        SortByAdded,
        SortByUpdated,
        SortByLastWatched,
        SortByMostWatched
    };

signals:
    void onToggleShowUpdated(bool enable);
    void onSortingOrderChanged(int sortOrder);
    void onUnwatchedCountChanged(int count);
    void onMarkAllAsWatched();
    void onBeforeAppearance();
    void onAppeared();
    void onBeforeDisappearance();
    void onDisappeared();
    void onChannelActivated(YTChannel *);
    void onVideoActivated(const QString &title, bool unwatched);
    void onMarkChannelWatched();
    void onChannelUnsubscribe();

public slots:
    void appear();
    void disappear();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void leaveEvent(QEvent *event);
    void paintEvent(QPaintEvent *event);

private slots:
    void itemEntered(const QModelIndex &index);
    void itemActivated(const QModelIndex &index);
    void showContextMenu(const QPoint &point);
    void toggleShowUpdated(bool enable);
    void setSortBy(SortBy sortBy);
    void setSortByName() { setSortBy(SortByName); }
    void setSortByUpdated() { setSortBy(SortByUpdated); }
    void setSortByAdded() { setSortBy(SortByAdded); }
    void setSortByLastWatched() { setSortBy(SortByLastWatched); }
    void setSortByMostWatched() { setSortBy(SortByMostWatched); }
    void markAllAsWatched();
    void unwatchedCountChanged(int count);

private:
    void updateView(bool transition = false);
    void setupActions();
    QAction *createAction(const QString &text, QActionGroup *sortGroup, bool isChecked, const char *slot);

    ChannelModel *channelsModel;
    QList<QAction*> statusActions;
    QAction *markAsWatchedAction;
};

#endif // CHANNELSVIEW_H
