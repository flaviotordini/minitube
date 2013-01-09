#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include <QtGui>

class Video;
class VideoSource;

enum DataRoles {
    ItemTypeRole = Qt::UserRole,
    VideoRole,
    ActiveTrackRole,
    DownloadItemRole,
    HoveredItemRole,
    DownloadButtonHoveredRole,
    DownloadButtonPressedRole,
    AuthorHoveredRole,
    AuthorPressedRole
};

enum ItemTypes {
    ItemTypeVideo = 1,
    ItemTypeShowMore
};

class PlaylistModel : public QAbstractListModel {

    Q_OBJECT

public:
    PlaylistModel(QWidget *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
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

    void setActiveRow( int row );
    bool rowExists( int row ) const { return (( row >= 0 ) && ( row < videos.size() ) ); }
    int activeRow() const { return m_activeRow; } // returns -1 if there is no active row
    int nextRow() const;
    int previousRow() const;
    void removeIndexes(QModelIndexList &indexes);
    int rowForVideo(Video* video);
    QModelIndex indexForVideo(Video* video);
    void move(QModelIndexList &indexes, bool up);

    Video* videoAt( int row ) const;
    Video* activeVideo() const;
    bool hasVideo(Video *video) const { return videos.contains(video); }

    VideoSource* getVideoSource() { return videoSource; }
    void setVideoSource(VideoSource *videoSource);
    void abortSearch();

public slots:
    void searchMore();
    void searchNeeded();
    void addVideo(Video* video);
    void searchFinished(int total);
    void searchError(QString message);
    void updateThumbnail();

    void setHoveredRow(int row);
    void clearHover();
    void enterAuthorHover();
    void exitAuthorHover();
    void enterAuthorPressed();
    void exitAuthorPressed();
    void updateAuthor();

signals:
    void activeRowChanged(int);
    void needSelectionFor(QList<Video*>);
    void haveSuggestions(const QStringList &suggestions);

private:
    void searchMore(int max);

    VideoSource *videoSource;
    bool searching;
    bool canSearchMore;

    QList<Video*> videos;
    int skip;
    int max;

    int m_activeRow;
    Video *m_activeVideo;

    QString errorMessage;

    int hoveredRow;
    bool authorHovered;
    bool authorPressed;
};

#endif
