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

#include "playlistmodel.h"
#include "videomimedata.h"
#include "videosource.h"
#include "ytsearch.h"
#include "video.h"
#include "searchparams.h"
#include "mediaview.h"

static const int maxItems = 50;
static const QString recentKeywordsKey = "recentKeywords";
static const QString recentChannelsKey = "recentChannels";

PlaylistModel::PlaylistModel(QWidget *parent) : QAbstractListModel(parent) {
    videoSource = 0;
    searching = false;
    canSearchMore = true;
    firstSearch = false;
    m_activeVideo = 0;
    m_activeRow = -1;
    startIndex = 1;
    max = 0;
    hoveredRow = -1;
    authorHovered = false;
    authorPressed = false;
}

int PlaylistModel::rowCount(const QModelIndex &/*parent*/) const {
    int count = videos.size();
    
    // add the message item
    if (videos.isEmpty() || !searching)
        count++;
    
    return count;
}

QVariant PlaylistModel::data(const QModelIndex &index, int role) const {
    
    int row = index.row();
    
    if (row == videos.size()) {
        
        QPalette palette;
        QFont boldFont;
        boldFont.setBold(true);
        
        switch (role) {
        case ItemTypeRole:
            return ItemTypeShowMore;
        case Qt::DisplayRole:
            if (!errorMessage.isEmpty()) return errorMessage;
            if (searching) return tr("Searching...");
            if (canSearchMore) return tr("Show %1 More").arg("").simplified();
            if (videos.isEmpty()) return tr("No videos");
            else return tr("No more videos");
        case Qt::TextAlignmentRole:
            return QVariant(int(Qt::AlignHCenter | Qt::AlignVCenter));
        case Qt::ForegroundRole:
            if (!errorMessage.isEmpty())
                return palette.color(QPalette::ToolTipText);
            else
                return palette.color(QPalette::Dark);
        case Qt::BackgroundColorRole:
            if (!errorMessage.isEmpty())
                return palette.color(QPalette::ToolTipBase);
            else
                return QVariant();
        case Qt::FontRole:
            return boldFont;
        default:
            return QVariant();
        }
        
    } else if (row < 0 || row >= videos.size())
        return QVariant();
    
    Video *video = videos.at(row);
    
    switch (role) {
    case ItemTypeRole:
        return ItemTypeVideo;
    case VideoRole:
        return QVariant::fromValue(QPointer<Video>(video));
    case ActiveTrackRole:
        return video == m_activeVideo;
    case Qt::DisplayRole:
        return video->title();
    case HoveredItemRole:
        return hoveredRow == index.row();
    case AuthorHoveredRole:
        return authorHovered;
    case AuthorPressedRole:
        return authorPressed;
    case Qt::StatusTipRole:
        return video->description();
    }
    
    return QVariant();
}

void PlaylistModel::setActiveRow(int row, bool notify) {
    if ( rowExists( row ) ) {
        
        m_activeRow = row;
        m_activeVideo = videoAt(row);
        
        int oldactiverow = m_activeRow;
        
        if ( rowExists( oldactiverow ) )
            emit dataChanged( createIndex( oldactiverow, 0 ), createIndex( oldactiverow, columnCount() - 1 ) );
        
        emit dataChanged( createIndex( m_activeRow, 0 ), createIndex( m_activeRow, columnCount() - 1 ) );
        if (notify) emit activeRowChanged(row);
        
    } else {
        m_activeRow = -1;
        m_activeVideo = 0;
    }

}

int PlaylistModel::nextRow() const {
    int nextRow = m_activeRow + 1;
    if (rowExists(nextRow))
        return nextRow;
    return -1;
}

int PlaylistModel::previousRow() const {
    int prevRow = m_activeRow - 1;
    if (rowExists(prevRow))
        return prevRow;
    return -1;
}

Video* PlaylistModel::videoAt( int row ) const {
    if ( rowExists( row ) )
        return videos.at( row );
    return 0;
}

Video* PlaylistModel::activeVideo() const {
    return m_activeVideo;
}

void PlaylistModel::setVideoSource(VideoSource *videoSource) {
    beginResetModel();
    while (!videos.isEmpty()) delete videos.takeFirst();
    videos.clear();
    m_activeVideo = 0;
    m_activeRow = -1;
    startIndex = 1;
    endResetModel();

    this->videoSource = videoSource;
    connect(videoSource, SIGNAL(gotVideos(QList<Video*>)),
            SLOT(addVideos(QList<Video*>)), Qt::UniqueConnection);
    connect(videoSource, SIGNAL(finished(int)),
            SLOT(searchFinished(int)), Qt::UniqueConnection);
    connect(videoSource, SIGNAL(error(QString)),
            SLOT(searchError(QString)), Qt::UniqueConnection);

    searchMore();
}

void PlaylistModel::searchMore(int max) {
    if (searching) return;
    searching = true;
    firstSearch = startIndex == 1;
    this->max = max;
    errorMessage.clear();
    videoSource->loadVideos(max, startIndex);
    startIndex += max;
}

void PlaylistModel::searchMore() {
    searchMore(maxItems);
}

void PlaylistModel::searchNeeded() {
    const int desiredRowsAhead = 10;
    int remainingRows = videos.size() - m_activeRow;
    if (remainingRows < desiredRowsAhead)
        searchMore(maxItems);
}

void PlaylistModel::abortSearch() {
    QMutexLocker locker(&mutex);
    beginResetModel();
    // while (!videos.isEmpty()) delete videos.takeFirst();
    // if (videoSource) videoSource->abort();
    videos.clear();
    searching = false;
    m_activeRow = -1;
    m_activeVideo = 0;
    startIndex = 1;
    endResetModel();
}

void PlaylistModel::searchFinished(int total) {
    qDebug() << __PRETTY_FUNCTION__ << total;
    searching = false;
    canSearchMore = videoSource->hasMoreVideos();

    // update the message item
    emit dataChanged( createIndex( maxItems, 0 ), createIndex( maxItems, columnCount() - 1 ) );

    if (!videoSource->getSuggestions().isEmpty())
        emit haveSuggestions(videoSource->getSuggestions());

    if (firstSearch && !videos.isEmpty())
        handleFirstVideo(videos.first());
}

void PlaylistModel::searchError(const QString &message) {
    errorMessage = message;
    // update the message item
    emit dataChanged( createIndex( maxItems, 0 ), createIndex( maxItems, columnCount() - 1 ) );
}

void PlaylistModel::addVideos(QList<Video*> newVideos) {
    if (newVideos.isEmpty()) return;
    beginInsertRows(QModelIndex(), videos.size(), videos.size() + newVideos.size() - 2);
    videos.append(newVideos);
    endInsertRows();
    foreach (Video* video, newVideos) {
        connect(video, SIGNAL(gotThumbnail()),
                SLOT(updateVideoSender()), Qt::UniqueConnection);
        video->loadThumbnail();
        qApp->processEvents();
    }
}

void PlaylistModel::handleFirstVideo(Video *video) {

    int currentVideoRow = rowForCloneVideo(MediaView::instance()->getCurrentVideoId());
    if (currentVideoRow != -1) setActiveRow(currentVideoRow, false);
    else {
        QSettings settings;
        if (!settings.value("manualplay", false).toBool())
            setActiveRow(0);
    }

    QSettings settings;
    if (!settings.value("manualplay", false).toBool()) {
        int newActiveRow = rowForCloneVideo(MediaView::instance()->getCurrentVideoId());
        if (newActiveRow != -1) setActiveRow(newActiveRow, false);
        else setActiveRow(0);
    }

    if (videoSource->metaObject()->className() == QLatin1String("YTSearch")) {

        static const int maxRecentElements = 10;

        YTSearch *search = dynamic_cast<YTSearch *>(videoSource);
        SearchParams *searchParams = search->getSearchParams();

        // save keyword
        QString query = searchParams->keywords();
        if (!query.isEmpty() && !searchParams->isTransient()) {
            if (query.startsWith("http://")) {
                // Save the video title
                query += "|" + videos.first()->title();
            }
            QStringList keywords = settings.value(recentKeywordsKey).toStringList();
            keywords.removeAll(query);
            keywords.prepend(query);
            while (keywords.size() > maxRecentElements)
                keywords.removeLast();
            settings.setValue(recentKeywordsKey, keywords);
        }

        // save channel
        QString channelId = searchParams->channelId();
        if (!channelId.isEmpty() && !searchParams->isTransient()) {
            QString value;
            if (!video->channelId().isEmpty() && video->channelId() != video->channelTitle())
                value = video->channelId() + "|" + video->channelTitle();
            else value = video->channelTitle();
            QStringList channels = settings.value(recentChannelsKey).toStringList();
            channels.removeAll(value);
            channels.removeAll(channelId);
            channels.prepend(value);
            while (channels.size() > maxRecentElements)
                channels.removeLast();
            settings.setValue(recentChannelsKey, channels);
        }
    }
}

void PlaylistModel::updateVideoSender() {
    Video *video = static_cast<Video *>(sender());
    if (!video) {
        qDebug() << "Cannot get sender";
        return;
    }
    int row = rowForVideo(video);
    emit dataChanged( createIndex( row, 0 ), createIndex( row, columnCount() - 1 ) );
}

void PlaylistModel::emitDataChanged() {
    QModelIndex index = createIndex(rowCount()-1, 0);
    emit dataChanged(index, index);
}

// --- item removal

/**
  * This function does not free memory
  */
bool PlaylistModel::removeRows(int position, int rows, const QModelIndex & /*parent*/) {
    beginRemoveRows(QModelIndex(), position, position+rows-1);
    for (int row = 0; row < rows; ++row) {
        videos.removeAt(position);
    }
    endRemoveRows();
    return true;
}

void PlaylistModel::removeIndexes(QModelIndexList &indexes) {
    QList<Video*> originalList(videos);
    QList<Video*> delitems;
    foreach (const QModelIndex &index, indexes) {
        if (index.row() >= originalList.size()) continue;
        Video* video = originalList.at(index.row());
        int idx = videos.indexOf(video);
        if (idx != -1) {
            beginRemoveRows(QModelIndex(), idx, idx);
            delitems.append(video);
            videos.removeAll(video);
            endRemoveRows();
        }
    }

    qDeleteAll(delitems);

}

// --- Sturm und drang ---



Qt::DropActions PlaylistModel::supportedDropActions() const {
    return Qt::CopyAction;
}

Qt::DropActions PlaylistModel::supportedDragActions() const {
    return Qt::CopyAction;
}

Qt::ItemFlags PlaylistModel::flags(const QModelIndex &index) const {
    if (index.isValid()) {
        if (index.row() == videos.size()) {
            // don't drag the "show more" item
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        } else return (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
    }
    return Qt::ItemIsDropEnabled;
}

QStringList PlaylistModel::mimeTypes() const {
    QStringList types;
    types << "application/x-minitube-video";
    return types;
}

QMimeData* PlaylistModel::mimeData( const QModelIndexList &indexes ) const {
    VideoMimeData* mime = new VideoMimeData();

    foreach( const QModelIndex &it, indexes ) {
        int row = it.row();
        if (row >= 0 && row < videos.size())
            mime->addVideo( videos.at( it.row() ) );
    }

    return mime;
}

bool PlaylistModel::dropMimeData(const QMimeData *data,
                                 Qt::DropAction action, int row, int column,
                                 const QModelIndex &parent) {
    if (action == Qt::IgnoreAction)
        return true;

    if (!data->hasFormat("application/x-minitube-video"))
        return false;

    if (column > 0)
        return false;

    int beginRow;
    if (row != -1)
        beginRow = row;
    else if (parent.isValid())
        beginRow = parent.row();
    else
        beginRow = rowCount(QModelIndex());

    const VideoMimeData* videoMimeData = dynamic_cast<const VideoMimeData*>( data );
    if(!videoMimeData ) return false;

    QList<Video*> droppedVideos = videoMimeData->videos();
    foreach( Video *video, droppedVideos) {
        
        // remove videos
        int videoRow = videos.indexOf(video);
        removeRows(videoRow, 1, QModelIndex());
        
        // and then add them again at the new position
        beginInsertRows(QModelIndex(), beginRow, beginRow);
        videos.insert(beginRow, video);
        endInsertRows();

    }

    // fix m_activeRow after all this
    m_activeRow = videos.indexOf(m_activeVideo);

    // let the MediaView restore the selection
    emit needSelectionFor(droppedVideos);

    return true;

}

int PlaylistModel::rowForCloneVideo(const QString &videoId) const {
    if (videoId.isEmpty()) return -1;
    for (int i = 0; i < videos.size(); ++i) {
        Video *v = videos.at(i);
        // qDebug() << "Comparing" << v->id() << videoId;
        if (v->id() == videoId) return i;
    }
    return -1;
}

int PlaylistModel::rowForVideo(Video* video) {
    return videos.indexOf(video);
}

QModelIndex PlaylistModel::indexForVideo(Video* video) {
    return createIndex(videos.indexOf(video), 0);
}

void PlaylistModel::move(QModelIndexList &indexes, bool up) {
    QList<Video*> movedVideos;

    foreach (const QModelIndex &index, indexes) {
        int row = index.row();
        if (row >= videos.size()) continue;
        // qDebug() << "index row" << row;
        Video *video = videoAt(row);
        movedVideos << video;
    }

    int end=up ? -1 : rowCount()-1, mod=up ? -1 : 1;
    foreach (Video *video, movedVideos) {

        int row = rowForVideo(video);
        if (row+mod==end) { end=row; continue; }
        // qDebug() << "video row" << row;
        removeRows(row, 1, QModelIndex());

        if (up) row--;
        else row++;

        beginInsertRows(QModelIndex(), row, row);
        videos.insert(row, video);
        endInsertRows();

    }

    emit needSelectionFor(movedVideos);

}

/* row hovering */

void PlaylistModel::setHoveredRow(int row) {
    int oldRow = hoveredRow;
    hoveredRow = row;
    emit dataChanged( createIndex( oldRow, 0 ), createIndex( oldRow, columnCount() - 1 ) );
    emit dataChanged( createIndex( hoveredRow, 0 ), createIndex( hoveredRow, columnCount() - 1 ) );
}

void PlaylistModel::clearHover() {
    int oldRow = hoveredRow;
    hoveredRow = -1;
    emit dataChanged( createIndex( oldRow, 0 ), createIndex( oldRow, columnCount() - 1) );
}

void PlaylistModel::updateHoveredRow() {
    emit dataChanged( createIndex( hoveredRow, 0 ), createIndex( hoveredRow, columnCount() - 1 ) );
}

/* clickable author */

void PlaylistModel::enterAuthorHover() {
    if (authorHovered) return;
    authorHovered = true;
    updateHoveredRow();
}

void PlaylistModel::exitAuthorHover() {
    if (!authorHovered) return;
    authorHovered = false;
    updateHoveredRow();
    setHoveredRow(hoveredRow);
}

void PlaylistModel::enterAuthorPressed() {
    if (authorPressed) return;
    authorPressed = true;
    updateHoveredRow();
}

void PlaylistModel::exitAuthorPressed() {
    if (!authorPressed) return;
    authorPressed = false;
    updateHoveredRow();
}
