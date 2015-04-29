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

#ifndef REGIONSVIEW_H
#define REGIONSVIEW_H

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif
#include "view.h"

struct YTRegion;

class RegionsView : public QWidget, public View {

    Q_OBJECT

public:
    RegionsView(QWidget *parent = 0);
    void appear();

signals:
    void regionChanged();

private slots:
    void buttonClicked();

private:
    void addRegion(const YTRegion &region);
    QGridLayout *layout;
    QPushButton *doneButton;

};

#endif // REGIONSVIEW_H
