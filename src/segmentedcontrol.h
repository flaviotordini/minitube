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

#ifndef SEGMENTEDCONTROL_H
#define SEGMENTEDCONTROL_H

#include <QtWidgets>

class SegmentedControl : public QWidget {

    Q_OBJECT

public:
    SegmentedControl(QWidget *parent = 0);
    QAction *addAction(QAction *action);
    bool setCheckedAction(int index);
    bool setCheckedAction(QAction *action);
    QSize minimumSizeHint(void) const;

signals:
    void checkedActionChanged(QAction & action);

protected:
    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void leaveEvent(QEvent *event);

private:
    void paintButton(QPainter *painter,
                    const QRect& rect,
                    const QAction *action);
    QAction *findHoveredAction(const QPoint& pos) const;
    int calculateButtonWidth() const;

    QVector<QAction *> actionList;
    QAction *checkedAction;
    QAction *hoveredAction;
    QAction *pressedAction;

    QColor borderColor;
    QColor backgroundColor;
    QColor selectedColor;
    QColor hoveredColor;
    QColor pressedColor;

};

#endif /* !SEGMENTEDCONTROL_H */
