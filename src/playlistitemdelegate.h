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

#ifndef PLAYLISTITEMDELEGATE_H
#define PLAYLISTITEMDELEGATE_H

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

class PlaylistItemDelegate : public QStyledItemDelegate {

    Q_OBJECT

public:
    PlaylistItemDelegate(QObject* parent, bool downloadInfo = false);
    ~PlaylistItemDelegate();

    QSize sizeHint( const QStyleOptionViewItem&, const QModelIndex& ) const;
    void paint( QPainter*, const QStyleOptionViewItem&, const QModelIndex& ) const;
    QRect downloadButtonRect(const QRect &line) const;
    QRect authorRect(const QModelIndex& index) const;

private:
    void createPlayIcon();
    void paintBody( QPainter*, const QStyleOptionViewItem&, const QModelIndex& ) const;
    void paintDownloadInfo( QPainter* painter,
                                        const QStyleOptionViewItem& option,
                                        const QModelIndex& index ) const;
    void paintActiveOverlay(QPainter *painter, const QStyleOptionViewItem& option, const QRect &line) const;
    void drawTime(QPainter *painter, const QString &time, const QRect &line) const;

    static const int THUMB_WIDTH;
    static const int THUMB_HEIGHT;
    static const int PADDING;

    QPixmap playIcon;
    QFont boldFont;
    QFont smallerFont;
    QFont smallerBoldFont;

    bool downloadInfo;
    QProgressBar *progressBar;

    mutable QRect lastAuthorRect;
    mutable QHash<int, QRect> authorRects;
};

#endif
