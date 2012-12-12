#ifndef PRETTYITEMDELEGATE_H
#define PRETTYITEMDELEGATE_H

#include <QModelIndex>
#include <QStyledItemDelegate>

class QPainter;
class QProgressBar;

class PlaylistItemDelegate : public QStyledItemDelegate {

    Q_OBJECT

public:
    PlaylistItemDelegate(QObject* parent, bool downloadInfo = false);
    ~PlaylistItemDelegate();

    QSize sizeHint( const QStyleOptionViewItem&, const QModelIndex& ) const;
    void paint( QPainter*, const QStyleOptionViewItem&, const QModelIndex& ) const;
    QRect downloadButtonRect(QRect line) const;
    QRect authorRect(const QModelIndex& index) const;

private:
    void createPlayIcon();
    void paintBody( QPainter*, const QStyleOptionViewItem&, const QModelIndex& ) const;
    void paintDownloadInfo( QPainter* painter,
                                        const QStyleOptionViewItem& option,
                                        const QModelIndex& index ) const;

    // active track painting
    void paintActiveOverlay( QPainter *painter, qreal x, qreal y, qreal w, qreal h ) const;
    void paintPlayIcon(QPainter *painter) const;

    //  Paints the video duration
    void drawTime(QPainter *painter, QString time, QRectF line) const;

    static const qreal THUMB_WIDTH;
    static const qreal THUMB_HEIGHT;
    static const qreal PADDING;

    QPixmap playIcon;
    QFont boldFont;
    QFont smallerFont;
    QFont smallerBoldFont;

    bool downloadInfo;
    QProgressBar *progressBar;

};

#endif
