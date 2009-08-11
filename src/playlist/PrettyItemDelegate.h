#ifndef PRETTYITEMDELEGATE_H
#define PRETTYITEMDELEGATE_H

#include <QModelIndex>
#include <QStyledItemDelegate>

class QPainter;




class PrettyItemDelegate : public QStyledItemDelegate {

    Q_OBJECT

public:
    PrettyItemDelegate( QObject* parent = 0 );
    ~PrettyItemDelegate();

    QSize sizeHint( const QStyleOptionViewItem&, const QModelIndex& ) const;
    void paint( QPainter*, const QStyleOptionViewItem&, const QModelIndex& ) const;

private:
    void paintBody( QPainter*, const QStyleOptionViewItem&, const QModelIndex& ) const;

    QPointF centerImage( const QPixmap&, const QRectF& ) const;

    /**
             * Paints a marker indicating the track is active
             */
    void paintActiveOverlay( QPainter *painter, qreal x, qreal y, qreal w, qreal h ) const;
    /**
          * Paints the video duration
          */
    void drawTime(QPainter *painter, QString time, QRectF line) const;

    static const qreal THUMB_WIDTH;
    static const qreal THUMB_HEIGHT;
    static const qreal MARGIN;
    static const qreal MARGINH;
    static const qreal MARGINBODY;
    static const qreal PADDING;

};


#endif

