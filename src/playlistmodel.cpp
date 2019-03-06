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
#include "mediaview.h"
#include "searchparams.h"
#include "video.h"
#include "videomimedata.h"
#include "videosource.h"
#include "ytsearch.h"

namespace {
const int maxItems = 50;
const QString recentKeywordsKey = "recentKeywords";
const QString recentChannelsKey = "recentChannels";
} // namespace

PlaylistModel::PlaylistModel(QWidget *parent) : QAbstractListModel(parent) {
    videoSource = nullptr;
    searching = false;
    canSearchMore = true;
    firstSearch = false;
    m_activeVideo = nullptr;
    m_activeRow = -1;
    startIndex = 1;
    max = 0;
    hoveredRow = -1;
    authorHovered = false;
    authorPressed = false;
}

int PlaylistModel::rowCount(const QModelIndex & /*parent*/) const {
    int count = videos.size();

    // add the message item
    if (videos.isEmpty() || !searching) count++;

    return count;
}

QVariant PlaylistModel::data(const QModelIndex &index, int role) const {
    int row = index.row();

    if (row == videos.size()) {
        QPalette palette;

        switch (role) {
        case ItemTypeRole:
            return ItemTypeShowMore;
        case Qt::DisplayRole:
            if (!errorMessage.isEmpty()) return errorMessage;
            if (searching) return QString(); // tr("Searching...");
            if (canSearchMore) return tr("Show %1 More").arg("").simplified();
            if (videos.isEmpty())
                return tr("No videos");
            else
                return tr("No more videos");
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
        return video->getTitle();
    case HoveredItemRole:
        return hoveredRow == index.row();
    case AuthorHoveredRole:
        return authorHovered;
    case AuthorPressedRole:
        return authorPressed;
        /*
    case Qt::StatusTipRole:
        return video->description();
        */
    }

    return QVariant();
}

void PlaylistModel::setActiveRow(int row, bool notify) {
    if (rowExists(row)) {
        m_activeRow = row;
        Video *previousVideo = m_activeVideo;
        m_activeVideo = videoAt(row);

        int oldactiverow = m_activeRow;

        if (rowExists(oldactiverow))
            emit dataChanged(createIndex(oldactiverow, 0),
                             createIndex(oldactiverow, columnCount() - 1));

        emit dataChanged(createIndex(m_activeRow, 0), createIndex(m_activeRow, columnCount() - 1));
        if (notify) emit activeVideoChanged(m_activeVideo, previousVideo);

    } else {
        m_activeRow = -1;
        m_activeVideo = nullptr;
    }
}

int PlaylistModel::nextRow() const {
    int nextRow = m_activeRow + 1;
    if (rowExists(nextRow)) return nextRow;
    return -1;
}

int PlaylistModel::previousRow() const {
    int prevRow = m_activeRow - 1;
    if (rowExists(prevRow)) return prevRow;
    return -1;
}

Video *PlaylistModel::videoAt(int row) const {
    if (rowExists(row)) return videos.at(row);
    return nullptr;
}

Video *PlaylistModel::activeVideo() const {
    return m_activeVideo;
}

void PlaylistModel::setVideoSource(VideoSource *videoSource) {
    beginResetModel();

    qDeleteAll(videos);
    videos.clear();

    qDeleteAll(deletedVideos);
    deletedVideos.clear();

    m_activeVideo = nullptr;
    m_activeRow = -1;
    startIndex = 1;
    endResetModel();

    this->videoSource = videoSource;
    connect(videoSource, SIGNAL(gotVideos(QVector<Video *>)), SLOT(addVideos(QVector<Video *>)),
            Qt::UniqueConnection);
    connect(videoSource, SIGNAL(finished(int)), SLOT(searchFinished(int)), Qt::UniqueConnection);
    connect(videoSource, SIGNAL(error(QString)), SLOT(searchError(QString)), Qt::UniqueConnection);
    connect(videoSource, &QObject::destroyed, this,
            [this, videoSource] {
                if (this->videoSource == videoSource) {
                    this->videoSource = nullptr;
                }
            },
            Qt::UniqueConnection);

    searchMore();
}

void PlaylistModel::searchMore(int max) {
    if (videoSource == nullptr || searching) return;
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
    if (remainingRows < desiredRowsAhead) searchMore(maxItems);
}

void PlaylistModel::abortSearch() {
    QMutexLocker locker(&mutex);
    beginResetModel();
    if (videoSource) videoSource->abort();
    qDeleteAll(videos);
    videos.clear();
    videos.squeeze();
    searching = false;
    m_activeRow = -1;
    m_activeVideo = nullptr;
    startIndex = 1;
    endResetModel();
}

void PlaylistModel::searchFinished(int total) {
    Q_UNUSED(total);
    searching = false;
    canSearchMore = videoSource->hasMoreVideos();

    // update the message item
    emit dataChanged(createIndex(maxItems, 0), createIndex(maxItems, columnCount() - 1));

    if (firstSearch && !videos.isEmpty()) handleFirstVideo(videos.at(0));
}

void PlaylistModel::searchError(const QString &message) {
    errorMessage = message;
    // update the message item
    emit dataChanged(createIndex(maxItems, 0), createIndex(maxItems, columnCount() - 1));
}

void PlaylistModel::addVideos(const QVector<Video *> &newVideos) {
    if (newVideos.isEmpty()) return;
    videos.reserve(videos.size() + newVideos.size());
    beginInsertRows(QModelIndex(), videos.size(), videos.size() + newVideos.size() - 2);
    videos.append(newVideos);
    endInsertRows();
    for (Video *video : newVideos) {
        connect(video, SIGNAL(gotThumbnail()), SLOT(updateVideoSender()), Qt::UniqueConnection);
        video->loadThumbnail();
    }
}

void PlaylistModel::handleFirstVideo(Video *video) {
    QSettings settings;
    int currentVideoRow = rowForCloneVideo(MediaView::instance()->getCurrentVideoId());
    if (currentVideoRow != -1)
        setActiveRow(currentVideoRow, false);
    else {
        if (!settings.value("manualplay", false).toBool()) setActiveRow(0);
    }

    if (videoSource->metaObject()->className() == QLatin1String("YTSearch")) {
        static const int maxRecentElements = 10;

        YTSearch *search = qobject_cast<YTSearch *>(videoSource);
        SearchParams *searchParams = search->getSearchParams();

        // save keyword
        QString query = searchParams->keywords();
        if (!query.isEmpty() && !searchParams->isTransient()) {
            if (query.startsWith("http://")) {
                // Save the video title
                query += "|" + videos.at(0)->getTitle();
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
            if (!video->getChannelId().isEmpty() &&
                video->getChannelId() != video->getChannelTitle())
                value = video->getChannelId() + "|" + video->getChannelTitle();
            else
                value = video->getChannelTitle();
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
    emit dataChanged(createIndex(row, 0), createIndex(row, columnCount() - 1));
}

void PlaylistModel::emitDataChanged() {
    QModelIndex index = createIndex(rowCount() - 1, 0);
    emit dataChanged(index, index);
}

// --- item removal

bool PlaylistModel::removeRows(int position, int rows, const QModelIndex & /*parent*/) {
    beginRemoveRows(QModelIndex(), position, position + rows - 1);
    for (int row = 0; row < rows; ++row) {
        Video *video = videos.takeAt(position);
    }
    endRemoveRows();
    return true;
}

void PlaylistModel::removeIndexes(QModelIndexList &indexes) {
    QVector<Video *> originalList(videos);
    for (const QModelIndex &index : indexes) {
        if (index.row() >= originalList.size()) continue;
        Video *video = originalList.at(index.row());
        int idx = videos.indexOf(video);
        if (idx != -1) {
            beginRemoveRows(QModelIndex(), idx, idx);
            deletedVideos.append(video);
            if (m_activeVideo == video) {
                m_activeVideo = nullptr;
                m_activeRow = -1;
            }
            videos.removeAll(video);
            endRemoveRows();
        }
    }
    videos.squeeze();
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
        } else
            return (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
    }
    return Qt::ItemIsDropEnabled;
}

QStringList PlaylistModel::mimeTypes() const {
    QStringList types;
    types << "application/x-minitube-video";
    return types;
}

QMimeData *PlaylistModel::mimeData(const QModelIndexList &indexes) const {
    VideoMimeData *mime = new VideoMimeData();

    for (const QModelIndex &it : indexes) {
        int row = it.row();
        if (row >= 0 && row < videos.size()) mime->addVideo(videos.at(it.row()));
    }

    return mime;
}

bool PlaylistModel::dropMimeData(const QMimeData *data,
                                 Qt::DropAction action,
                                 int row,
                                 int column,
                                 const QModelIndex &parent) {
    if (action == Qt::IgnoreAction) return true;

    if (!data->hasFormat("application/x-minitube-video")) return false;

    if (column > 0) return false;

    int beginRow;
    if (row != -1)
        beginRow = row;
    else if (parent.isValid())
        beginRow = parent.row();
    else
        beginRow = rowCount(QModelIndex());

    const VideoMimeData *videoMimeData = qobject_cast<const VideoMimeData *>(data);
    if (!videoMimeData) return false;

    const QVector<Video *> &droppedVideos = videoMimeData->getVideos();
    for (Video *video : droppedVideos) {
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
        if (v->getId() == videoId) return i;
    }
    return -1;
}

int PlaylistModel::rowForVideo(Video *video) {
    return videos.indexOf(video);
}

QModelIndex PlaylistModel::indexForVideo(Video *video) {
    return createIndex(videos.indexOf(video), 0);
}

void PlaylistModel::move(QModelIndexList &indexes, bool up) {
    QVector<Video *> movedVideos;

    for (const QModelIndex &index : indexes) {
        int row = index.row();
        if (row >= videos.size()) continue;
        // qDebug() << "index row" << row;
        Video *video = videoAt(row);
        movedVideos << video;
    }

    int end = up ? -1 : rowCount() - 1, mod = up ? -1 : 1;
    for (Video *video : movedVideos) {
        int row = rowForVideo(video);
        if (row + mod == end) {
            end = row;
            continue;
        }
        // qDebug() << "video row" << row;
        removeRows(row, 1, QModelIndex());

        if (up)
            row--;
        else
            row++;

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
    emit dataChanged(createIndex(oldRow, 0), createIndex(oldRow, columnCount() - 1));
    emit dataChanged(createIndex(hoveredRow, 0), createIndex(hoveredRow, columnCount() - 1));
}

void PlaylistModel::clearHover() {
    int oldRow = hoveredRow;
    hoveredRow = -1;
    emit dataChanged(createIndex(oldRow, 0), createIndex(oldRow, columnCount() - 1));
}

void PlaylistModel::updateHoveredRow() {
    emit dataChanged(createIndex(hoveredRow, 0), createIndex(hoveredRow, columnCount() - 1));
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
