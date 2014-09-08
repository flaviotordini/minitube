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

#include "downloadmodel.h"
#include "downloadmanager.h"
#include "downloaditem.h"
#include "video.h"
#include "playlistmodel.h"

DownloadModel::DownloadModel(DownloadManager *downloadManager, QObject *parent) :
        QAbstractListModel(parent),
        downloadManager(downloadManager) {
    hoveredRow = -1;
    playIconHovered = false;
    playIconPressed = false;
}

int DownloadModel::rowCount(const QModelIndex &/*parent*/) const {
    return downloadManager->getItems().size();
}

QVariant DownloadModel::data(const QModelIndex &index, int role) const {

    int row = index.row();
    if (row < 0 || row >= rowCount()) return QVariant();

    QList<DownloadItem*> items = downloadManager->getItems();
    if (items.isEmpty()) return QVariant();

    switch (role) {
    case ItemTypeRole:
        return ItemTypeVideo;
    case VideoRole:
        return QVariant::fromValue(QPointer<Video>(items.at(row)->getVideo()));
    case DownloadItemRole:
        return QVariant::fromValue(QPointer<DownloadItem>(items.at(row)));
    case ActiveTrackRole:
        return false;
    case HoveredItemRole:
        return hoveredRow == index.row();
    case DownloadButtonHoveredRole:
        return playIconHovered;
    case DownloadButtonPressedRole:
        return playIconPressed;
    }

    return QVariant();
}

void DownloadModel::sendReset() {
    beginResetModel();
    endResetModel();
}

void DownloadModel::setHoveredRow(int row) {
    int oldRow = hoveredRow;
    hoveredRow = row;
    emit dataChanged( createIndex( oldRow, 0 ), createIndex( oldRow, columnCount() - 1 ) );
    emit dataChanged( createIndex( hoveredRow, 0 ), createIndex( hoveredRow, columnCount() - 1 ) );
}

void DownloadModel::clearHover() {
    emit dataChanged( createIndex( hoveredRow, 0 ), createIndex( hoveredRow, columnCount() - 1 ) );
    hoveredRow = -1;
}

void DownloadModel::enterPlayIconHover() {
    if (playIconHovered) return;
    playIconHovered = true;
    updatePlayIcon();
}

void DownloadModel::exitPlayIconHover() {
    if (!playIconHovered) return;
    playIconHovered = false;
    updatePlayIcon();
    setHoveredRow(hoveredRow);
}

void DownloadModel::enterPlayIconPressed() {
    if (playIconPressed) return;
    playIconPressed = true;
    updatePlayIcon();
}

void DownloadModel::exitPlayIconPressed() {
    if (!playIconPressed) return;
    playIconPressed = false;
    updatePlayIcon();
}

void DownloadModel::updatePlayIcon() {
    emit dataChanged( createIndex( hoveredRow, 0 ), createIndex( hoveredRow, columnCount() - 1 ) );
}
