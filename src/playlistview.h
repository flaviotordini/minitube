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

#ifndef PLAYLISTVIEW_H
#define PLAYLISTVIEW_H

#include <QtWidgets>

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
