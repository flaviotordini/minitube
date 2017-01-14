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

#ifndef CHANNELLISTVIEW_H
#define CHANNELLISTVIEW_H

#include <QtWidgets>

class ChannelListView : public QListView {

    Q_OBJECT

public:
    ChannelListView();
    void setErrorMessage(const QString &value) { errorMessage = value; }
    void clearErrorMessage() { errorMessage.clear(); }

signals:
    void contextMenu(QPoint point);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);

private:
    QString errorMessage;

};

#endif // CHANNELLISTVIEW_H
