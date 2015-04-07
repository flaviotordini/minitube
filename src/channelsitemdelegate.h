#ifndef CHANNELSITEMDELEGATE_H
#define CHANNELSITEMDELEGATE_H

#include <QtGui>

class ChannelsItemDelegate : public QStyledItemDelegate {

    Q_OBJECT

public:
    ChannelsItemDelegate(QObject* parent = 0);
    QSize sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const;
    void paint(QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const;

private:
    void paintChannel(QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const;
    void paintAggregate(QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const;
    void paintUnwatched(QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const;
    void paintBadge(QPainter *painter, const QRect &line, const QString &text) const;
    void drawItem(QPainter*, const QRect &line, const QPixmap &thumbnail, const QString &name) const;

};

#endif // CHANNELSITEMDELEGATE_H
