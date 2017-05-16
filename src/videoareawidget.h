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

#include <QtWidgets>

class Video;
class LoadingWidget;
class PlaylistModel;
class SnapshotPreview;

class VideoAreaWidget : public QWidget {

    Q_OBJECT

public:
    VideoAreaWidget(QWidget *parent = 0);
    void setVideoWidget(QWidget *videoWidget);
    void setLoadingWidget(LoadingWidget *loadingWidget);
    void showLoading(Video* video);
    void showVideo();
    void showError(const QString &message);
    void clear();
    void setListModel(PlaylistModel *listModel) {
        this->listModel = listModel;
    }
#ifdef APP_SNAPSHOT
    void showSnapshotPreview(const QPixmap &pixmap);
#endif
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
    void showContextMenu(const QPoint &point);
#ifdef APP_SNAPSHOT
    void hideSnapshotPreview();
#endif

private:
    QStackedLayout *stackedLayout;
    QWidget *videoWidget;
    LoadingWidget *loadingWidget;

#ifdef APP_SNAPSHOT
    SnapshotPreview *snapshotPreview;
#endif

    PlaylistModel *listModel;
    QLabel *messageLabel;

    QPoint dragPosition;
};

#endif // VIDEOAREAWIDGET_H
