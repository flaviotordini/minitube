#include "ListModel.h"
#include "videomimedata.h"

#define MAX_ITEMS 10
static const QString recentKeywordsKey = "recentKeywords";

ListModel::ListModel(QWidget *parent) : QAbstractListModel(parent) {
    youtubeSearch = 0;
    searching = false;
    canSearchMore = true;
    m_activeVideo = 0;
    m_activeRow = -1;
    skip = 1;
}

ListModel::~ListModel() {
    delete youtubeSearch;
}

int ListModel::rowCount(const QModelIndex &/*parent*/) const {
    int count = videos.size();
    
    // add the message item
    if (videos.isEmpty() || !searching)
        count++;
    
    return count;
}

QVariant ListModel::data(const QModelIndex &index, int role) const {
    
    int row = index.row();
    
    if (row == videos.size()) {
        
        QPalette palette;
        QFont boldFont;
        boldFont.setBold(true);
        
        switch (role) {
        case ItemTypeRole:
            return ItemTypeShowMore;
        case Qt::DisplayRole:
        case Qt::StatusTipRole:
            if (!errorMessage.isEmpty()) return errorMessage;
            if (searching) return tr("Searching...");
            if (canSearchMore) return tr("Show %1 More").arg(MAX_ITEMS);
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
    case Qt::StatusTipRole:
        return video->title();
        /*
        case Qt::ToolTipRole:
          
            QString tooltip;
            if (!element.firstChildElement().text().isEmpty()) {
                tooltip.append(QString("<b>").append(element.firstChildElement().text()).append("</b><br/>"));
            }
            if (!fromDate.isEmpty()) {
                tooltip.append("<i>Pubblicato il</i> ").append(fromDate);
            }
            if (!toDate.isEmpty()) {
                tooltip.append("<br/><i>Scadenza</i>: ").append(toDate);
            }
            tooltip.append("<br/><i>Tipo</i>: ").append(typeName)
                .append("<br/><i>Id</i>: ").appen    QFont boldFont;
    boldFont.setBold(true);d(id);
            return tooltip;
            */
        
        // case StreamUrlRole:
        // return video->streamUrl();
    }
    
    return QVariant();
}

void ListModel::setActiveRow( int row) {
    if ( rowExists( row ) ) {
        
        m_activeRow = row;
        m_activeVideo = videoAt(row);
        
        // setStateOfRow( row, Item::Played );
        
        int oldactiverow = m_activeRow;
        
        if ( rowExists( oldactiverow ) )
            emit dataChanged( createIndex( oldactiverow, 0 ), createIndex( oldactiverow, columnCount() - 1 ) );
        
        emit dataChanged( createIndex( m_activeRow, 0 ), createIndex( m_activeRow, columnCount() - 1 ) );
        emit activeRowChanged(row);
        
    } else {
        m_activeRow = -1;
        m_activeVideo = 0;
    }

}

int ListModel::nextRow() const {
    int nextRow = m_activeRow + 1;
    if (rowExists(nextRow))
        return nextRow;
    return -1;
}

Video* ListModel::videoAt( int row ) const {
    if ( rowExists( row ) )
        return videos.at( row );
    return 0;
}

Video* ListModel::activeVideo() const {
    return m_activeVideo;
}

void ListModel::search(SearchParams *searchParams) {

    // delete current videos
    while (!videos.isEmpty())
        delete videos.takeFirst();
    m_activeVideo = 0;
    m_activeRow = -1;
    skip = 1;
    errorMessage.clear();
    reset();

    // (re)initialize the YouTubeSearch
    if (youtubeSearch) delete youtubeSearch;
    youtubeSearch = new YouTubeSearch();
    connect(youtubeSearch, SIGNAL(gotVideo(Video*)), this, SLOT(addVideo(Video*)));
    connect(youtubeSearch, SIGNAL(finished(int)), this, SLOT(searchFinished(int)));
    connect(youtubeSearch, SIGNAL(error(QString)), this, SLOT(searchError(QString)));

    this->searchParams = searchParams;
    searching = true;
    youtubeSearch->search(searchParams, MAX_ITEMS, skip);
    skip += MAX_ITEMS;
}

void ListModel::searchMore(int max) {
    if (searching) return;
    searching = true;
    errorMessage.clear();
    youtubeSearch->search(searchParams, max, skip);
    skip += max;
}

void ListModel::searchMore() {
    searchMore(MAX_ITEMS);
}

void ListModel::searchNeeded() {
    int remainingRows = videos.size() - m_activeRow;
    int rowsNeeded = MAX_ITEMS - remainingRows;
    if (rowsNeeded > 0)
        searchMore(rowsNeeded);
}

void ListModel::abortSearch() {
    while (!videos.isEmpty())
        delete videos.takeFirst();
    reset();
    youtubeSearch->abort();
    searching = false;
}

void ListModel::searchFinished(int total) {
    searching = false;
    canSearchMore = total > 0;

    // update the message item
    emit dataChanged( createIndex( MAX_ITEMS, 0 ), createIndex( MAX_ITEMS, columnCount() - 1 ) );
}

void ListModel::searchError(QString message) {
    errorMessage = message;
    // update the message item
    emit dataChanged( createIndex( MAX_ITEMS, 0 ), createIndex( MAX_ITEMS, columnCount() - 1 ) );
}

void ListModel::addVideo(Video* video) {
    
    connect(video, SIGNAL(gotThumbnail()), this, SLOT(updateThumbnail()));

    beginInsertRows(QModelIndex(), videos.size(), videos.size());
    videos << video;
    endInsertRows();
    
    // first result!
    if (videos.size() == 1) {
        // autoplay
        setActiveRow(0);

        // save keyword
        QString query = searchParams->keywords();
        QSettings settings;
        QStringList keywords = settings.value(recentKeywordsKey).toStringList();
        keywords.removeAll(query);
        keywords.prepend(query);
        while (keywords.size() > 10)
            keywords.removeLast();
        settings.setValue(recentKeywordsKey, keywords);
    }

}

void ListModel::updateThumbnail() {

    Video *video = static_cast<Video *>(sender());
    if (!video) {
        qDebug() << "Cannot get sender";
        return;
    }

    int row = rowForVideo(video);
    emit dataChanged( createIndex( row, 0 ), createIndex( row, columnCount() - 1 ) );

}

// --- item removal

/**
  * This function does not free memory
  */
bool ListModel::removeRows(int position, int rows, const QModelIndex & /*parent*/) {
    beginRemoveRows(QModelIndex(), position, position+rows-1);
    for (int row = 0; row < rows; ++row) {
        videos.removeAt(position);
    }
    endRemoveRows();
    return true;
}

void ListModel::removeIndexes(QModelIndexList &indexes) {
    QList<Video*> originalList(videos);
    QList<Video*> delitems;
    foreach (QModelIndex index, indexes) {
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



Qt::DropActions ListModel::supportedDropActions() const {
    return Qt::MoveAction;
}

Qt::ItemFlags ListModel::flags(const QModelIndex &index) const {
    Qt::ItemFlags defaultFlags = QAbstractListModel::flags(index);

    if (index.isValid()) {
        if (index.row() == videos.size()) {
            // don't drag the "show 10 more" item
            return defaultFlags;
        } else
            return ( defaultFlags | Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled );
    } else
        return Qt::ItemIsDropEnabled | defaultFlags;
}

QStringList ListModel::mimeTypes() const {
    QStringList types;
    types << "application/x-minitube-video";
    return types;
}

QMimeData* ListModel::mimeData( const QModelIndexList &indexes ) const {
    VideoMimeData* mime = new VideoMimeData();

    foreach( const QModelIndex &it, indexes ) {
        int row = it.row();
        if (row >= 0 && row < videos.size())
            mime->addVideo( videos.at( it.row() ) );
    }

    return mime;
}

bool ListModel::dropMimeData(const QMimeData *data,
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

int ListModel::rowForVideo(Video* video) {
    return videos.indexOf(video);
}

QModelIndex ListModel::indexForVideo(Video* video) {
    return createIndex(videos.indexOf(video), 0);
}

void ListModel::move(QModelIndexList &indexes, bool up) {
    QList<Video*> movedVideos;

    foreach (QModelIndex index, indexes) {
        int row = index.row();
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
