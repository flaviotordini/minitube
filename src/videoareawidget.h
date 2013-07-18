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

#ifndef VIDEOAREAWIDGET_H
#define VIDEOAREAWIDGET_H

#include <QWidget>
#include "video.h"
#include "loadingwidget.h"
#include "playlistmodel.h"

class VideoAreaWidget : public QWidget {

    Q_OBJECT

public:
    VideoAreaWidget(QWidget *parent = 0);
    void setVideoWidget(QWidget *videoWidget);
    void setLoadingWidget(LoadingWidget *loadingWidget);
    void showLoading(Video* video);
    void showVideo();
    void showError(QString message);
    void clear();
    void setListModel(PlaylistModel *listModel) {
        this->listModel = listModel;
    }
    // void showSnapshotPreview(QPixmap pixmap);
    bool isVideoShown() { return stackedLayout->currentWidget() == videoWidget; }

signals:
    void doubleClicked();
    void rightClicked();

protected:
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

private slots:
    // void hideSnapshotPreview();

private:
    QStackedLayout *stackedLayout;
    QWidget *videoWidget;
    LoadingWidget *loadingWidget;
    PlaylistModel *listModel;
    QLabel *messageLabel;
    // QLabel *snapshotPreview;
    QPoint dragPosition;

};

#endif // VIDEOAREAWIDGET_H
