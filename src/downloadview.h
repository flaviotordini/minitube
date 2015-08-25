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

#ifndef DOWNLOADVIEW_H
#define DOWNLOADVIEW_H

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif
#include "view.h"

class SegmentedControl;
class DownloadModel;
class DownloadListView;
class DownloadSettings;

class DownloadView : public View {

    Q_OBJECT

public:
    DownloadView(QWidget *parent);
    void appear();
    void disappear();
    QHash<QString, QVariant> metadata() {
        QHash<QString, QVariant> metadata;
        metadata.insert("title", tr("Downloads"));
        return metadata;
    }

public slots:
    void itemEntered(const QModelIndex &index);
    void buttonPushed(QModelIndex index);

private:
    SegmentedControl *bar;
    DownloadListView *listView;
    DownloadModel *listModel;
    QTimer *updateTimer;
    DownloadSettings *downloadSettings;

};

#endif // DOWNLOADVIEW_H
