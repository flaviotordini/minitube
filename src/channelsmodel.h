#ifndef CHANNELSMODEL_H
#define CHANNELSMODEL_H

#include <QtSql>

class YTUser;

class ChannelsModel : public QSqlQueryModel {

    Q_OBJECT

public:
    ChannelsModel(QObject *parent = 0);
    QVariant data(const QModelIndex &item, int role) const;
    void setHoveredRow(int row);
    YTUser* userForIndex(const QModelIndex &index) const;

    enum DataRoles {
        ItemTypeRole = Qt::UserRole,
        DataObjectRole,
        ActiveItemRole,
        HoveredItemRole
    };

    enum ItemTypes {
        ItemChannel = 1,
        ItemFolder,
        ItemAggregate
    };

public slots:
    void clearHover();

protected:
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QStringList mimeTypes() const;
    Qt::DropActions supportedDropActions() const;
    Qt::DropActions supportedDragActions() const;
    QMimeData* mimeData( const QModelIndexList &indexes ) const;

private:
    int hoveredRow;
};

#endif // CHANNELSMODEL_H
