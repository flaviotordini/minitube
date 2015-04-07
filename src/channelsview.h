#ifndef CHANNELSVIEW_H
#define CHANNELSVIEW_H

#include <QtGui>
#include "view.h"

class VideoSource;
class ChannelsModel;

class ChannelsView : public QListView, public View {

    Q_OBJECT

public:
    ChannelsView(QWidget *parent = 0);
    
signals:
    void activated(VideoSource *videoSource);

public slots:
    void appear();
    void disappear();

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void leaveEvent(QEvent *event);
    void paintEvent(QPaintEvent *event);

private:
    enum SortBy {
        SortByName = 0,
        SortByAdded,
        SortByUpdated,
        SortByLastWatched,
        SortByMostWatched
    };

private slots:
    void itemEntered(const QModelIndex &index);
    void itemActivated(const QModelIndex &index);
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
    void updateQuery(bool transition = false);
    void setupActions();

    ChannelsModel *channelsModel;
    QList<QAction*> statusActions;
    bool showUpdated;
    SortBy sortBy;
    QString errorMessage;
    QAction *markAsWatchedAction;

};

#endif // CHANNELSVIEW_H
