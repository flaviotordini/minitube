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
    void showSnapshotPreview(QPixmap pixmap);

signals:
    void doubleClicked();
    void rightClicked();

protected:
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

private slots:
    void hideSnapshotPreview();

private:
    QStackedLayout *stackedLayout;
    QWidget *videoWidget;
    LoadingWidget *loadingWidget;
    PlaylistModel *listModel;
    QLabel *messageLabel;
    QLabel *snapshotPreview;

};

#endif // VIDEOAREAWIDGET_H
