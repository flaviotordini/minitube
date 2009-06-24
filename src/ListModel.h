#ifndef LISTMODEL_H
#define LISTMODEL_H

#include "video.h"
#include "youtubesearch.h"
#include "searchparams.h"

enum DataRoles {
    ItemTypeRole = Qt::UserRole,
    VideoRole,
    ActiveTrackRole
};

enum ItemTypes {
    ItemTypeVideo = 1,
    ItemTypeShowMore
};

class ListModel : public QAbstractListModel {

    Q_OBJECT

public:

    ListModel(QWidget *parent);
    ~ListModel();

    // inherited from QAbstractListModel
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    // int rowCount( const QModelIndex& parent = QModelIndex() ) const { Q_UNUSED( parent ); return videos.size(); }
    int columnCount( const QModelIndex& parent = QModelIndex() ) const { Q_UNUSED( parent ); return 4; }
    QVariant data(const QModelIndex &index, int role) const;
    bool removeRows(int position, int rows, const QModelIndex &parent);

    Qt::ItemFlags flags(const QModelIndex &index) const;
    QStringList mimeTypes() const;
    Qt::DropActions supportedDropActions() const;
    QMimeData* mimeData( const QModelIndexList &indexes ) const;
    bool dropMimeData(const QMimeData *data,
                      Qt::DropAction action, int row, int column,
                      const QModelIndex &parent);

    // custom methods
    void setActiveRow( int row );
    bool rowExists( int row ) const { return (( row >= 0 ) && ( row < videos.size() ) ); }
    int activeRow() const { return m_activeRow; } // returns -1 if there is no active row
    int nextRow() const;
    void removeIndexes(QModelIndexList &indexes);
    int rowForVideo(Video* video);
    QModelIndex indexForVideo(Video* video);
    void move(QModelIndexList &indexes, bool up);

    Video* videoAt( int row ) const;
    Video* activeVideo() const;

    // video search methods
    void search(SearchParams *searchParams);
    void abortSearch();


public slots:
    void searchMore();
    void searchNeeded();
    void addVideo(Video* video);
    void searchFinished(int total);
    void updateThumbnail();

signals:
    void activeRowChanged(int);
    void needSelectionFor(QList<Video*>);

private:
    void searchMore(int max);

    YouTubeSearch *youtubeSearch;
    SearchParams *searchParams;
    bool searching;
    bool canSearchMore;

    QList<Video*> videos;
    int skip;

    // the row being played
    int m_activeRow;
    Video *m_activeVideo;

};

#endif
