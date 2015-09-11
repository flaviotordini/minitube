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

#include "channelitemdelegate.h"
#include "channelmodel.h"
#include "ytchannel.h"
#include "fontutils.h"
#include "channelaggregator.h"
#include "painterutils.h"
#include "iconutils.h"

static const int ITEM_WIDTH = 128;
static const int ITEM_HEIGHT = 128;
static const int THUMB_WIDTH = 88;
static const int THUMB_HEIGHT = 88;

ChannelItemDelegate::ChannelItemDelegate(QObject *parent) : QStyledItemDelegate(parent) {

}

QSize ChannelItemDelegate::sizeHint(const QStyleOptionViewItem& /*option*/,
                                     const QModelIndex& /*index*/ ) const {
    return QSize(ITEM_WIDTH, ITEM_HEIGHT);
}

void ChannelItemDelegate::paint( QPainter* painter,
                                  const QStyleOptionViewItem& option,
                                  const QModelIndex& index ) const {
    const int itemType = index.data(ChannelModel::ItemTypeRole).toInt();
    if (itemType == ChannelModel::ItemChannel)
        paintChannel(painter, option, index);
    else if (itemType == ChannelModel::ItemAggregate)
        paintAggregate(painter, option, index);
    else if (itemType == ChannelModel::ItemUnwatched)
        paintUnwatched(painter, option, index);
    else
        QStyledItemDelegate::paint(painter, option, index);
}

void ChannelItemDelegate::paintAggregate(QPainter* painter,
                                          const QStyleOptionViewItem& option,
                                          const QModelIndex& index) const {
    Q_UNUSED(index);
    painter->save();

    painter->translate(option.rect.topLeft());
    const QRect line(0, 0, option.rect.width(), option.rect.height());

    static const QPixmap thumbnail = IconUtils::pixmap(":/images/channels.png");

    QString name = tr("All Videos");

    drawItem(painter, line, thumbnail, name);

    painter->restore();
}

void ChannelItemDelegate::paintUnwatched(QPainter* painter,
                                          const QStyleOptionViewItem& option,
                                          const QModelIndex& index) const {
    Q_UNUSED(index);
    painter->save();

    painter->translate(option.rect.topLeft());
    const QRect line(0, 0, option.rect.width(), option.rect.height());

    static const QPixmap thumbnail = IconUtils::pixmap(":/images/unwatched.png");

    QString name = tr("Unwatched Videos");

    drawItem(painter, line, thumbnail, name);

    int notifyCount = ChannelAggregator::instance()->getUnwatchedCount();
    QString notifyText = QString::number(notifyCount);
    if (notifyCount > 0) paintBadge(painter, line, notifyText);

    painter->restore();
}

void ChannelItemDelegate::paintChannel(QPainter* painter,
                                        const QStyleOptionViewItem& option,
                                        const QModelIndex& index) const {
    YTChannel *channel = index.data(ChannelModel::DataObjectRole).value<YTChannelPointer>().data();
    if (!channel) return;

    painter->save();

    painter->translate(option.rect.topLeft());
    const QRect line(0, 0, option.rect.width(), option.rect.height());

    // const bool isActive = index.data( ActiveItemRole ).toBool();
    // const bool isHovered = index.data(ChannelsModel::HoveredItemRole ).toBool();
    // const bool isSelected = option.state & QStyle::State_Selected;

    QPixmap thumbnail = channel->getThumbnail();
    if (thumbnail.isNull()) {
        channel->loadThumbnail();
        painter->restore();
        return;
    }

    QString name = channel->getDisplayName();
    drawItem(painter, line, thumbnail, name);

    int notifyCount = channel->getNotifyCount();
    if (notifyCount > 0)
        paintBadge(painter, line, QString::number(notifyCount));

    painter->restore();
}

void ChannelItemDelegate::drawItem(QPainter *painter,
                                    const QRect &line,
                                    const QPixmap &thumbnail,
                                    const QString &name) const {
    painter->drawPixmap((line.width() - THUMB_WIDTH) / 2, 8, thumbnail);

    QRect nameBox = line;
    nameBox.adjust(0, 0, 0, -THUMB_HEIGHT - 16);
    nameBox.translate(0, line.height() - nameBox.height());
    bool tooBig = false;

    QRect textBox = painter->boundingRect(nameBox,
                                          Qt::AlignTop | Qt::AlignHCenter | Qt::TextWordWrap,
                                          name);
    if (textBox.height() > nameBox.height() || textBox.width() > nameBox.width()) {
        painter->setFont(FontUtils::small());
        textBox = painter->boundingRect(nameBox,
                                        Qt::AlignTop | Qt::AlignHCenter | Qt::TextWordWrap,
                                        name);
        if (textBox.height() > nameBox.height()) {
            painter->setClipRect(nameBox);
            tooBig = true;
        }
    }
    if (tooBig)
        painter->drawText(nameBox, Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap, name);
    else
        painter->drawText(textBox, Qt::AlignCenter | Qt::TextWordWrap, name);
}

void ChannelItemDelegate::paintBadge(QPainter *painter,
                                              const QRect &line,
                                              const QString &text) const {
    const int topLeft = (line.width() + THUMB_WIDTH) / 2;
    painter->save();
    painter->translate(topLeft, 0);
    painter->setClipping(false);
    PainterUtils::paintBadge(painter, text, true);
    painter->restore();
}
