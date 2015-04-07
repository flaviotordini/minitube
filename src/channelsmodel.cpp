#include "channelsmodel.h"
#include "ytuser.h"
#include "mainwindow.h"

ChannelsModel::ChannelsModel(QObject *parent) : QSqlQueryModel(parent) {
    hoveredRow = -1;
}

QVariant ChannelsModel::data(const QModelIndex &index, int role) const {

    YTUser* user = 0;

    switch (role) {

    case ChannelsModel::ItemTypeRole:
        return ChannelsModel::ItemChannel;
        break;

    case ChannelsModel::DataObjectRole:
        user = userForIndex(index);
        return QVariant::fromValue(QPointer<YTUser>(user));
        break;

    case ChannelsModel::HoveredItemRole:
        return hoveredRow == index.row();
        break;

    case Qt::StatusTipRole:
        user = userForIndex(index);
        return user->getDescription();

    }

    return QVariant();
}

YTUser* ChannelsModel::userForIndex(const QModelIndex &index) const {
    return YTUser::forId(QSqlQueryModel::data(QSqlQueryModel::index(index.row(), 0)).toString());
}

void ChannelsModel::setHoveredRow(int row) {
    int oldRow = hoveredRow;
    hoveredRow = row;
    emit dataChanged( createIndex( oldRow, 0 ), createIndex( oldRow, columnCount() - 1 ) );
    emit dataChanged( createIndex( hoveredRow, 0 ), createIndex( hoveredRow, columnCount() - 1 ) );
}

void ChannelsModel::clearHover() {
    emit dataChanged( createIndex( hoveredRow, 0 ), createIndex( hoveredRow, columnCount() - 1 ) );
    hoveredRow = -1;
}

// --- Sturm und drang ---


Qt::DropActions ChannelsModel::supportedDragActions() const {
    return Qt::CopyAction;
}

Qt::DropActions ChannelsModel::supportedDropActions() const {
    return Qt::CopyAction;
}

Qt::ItemFlags ChannelsModel::flags(const QModelIndex &index) const {
    if (index.isValid())
        return Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
    else return 0;
}

QStringList ChannelsModel::mimeTypes() const {
    QStringList types;
    types << "x-minitube/channel";
    return types;
}

QMimeData* ChannelsModel::mimeData( const QModelIndexList &indexes ) const {

    /* TODO
    UserMimeData* mime = new TrackMimeData();

    foreach( const QModelIndex &index, indexes ) {
        Item *item = userForIndex(index);
        if (item) {
            mime->addTracks(item->getTracks());
        }
    }

    return mime;
    */
}
