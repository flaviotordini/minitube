#include "channelsitemdelegate.h"
#include "channelmodel.h"
#include "ytuser.h"
#include "fontutils.h"
#include "channelaggregator.h"
#include "painterutils.h"

static const int ITEM_WIDTH = 128;
static const int ITEM_HEIGHT = 128;
static const int THUMB_WIDTH = 88;
static const int THUMB_HEIGHT = 88;

ChannelsItemDelegate::ChannelsItemDelegate(QObject *parent) : QStyledItemDelegate(parent) {

}

QSize ChannelsItemDelegate::sizeHint(const QStyleOptionViewItem& /*option*/,
                                     const QModelIndex& /*index*/ ) const {
    return QSize(ITEM_WIDTH, ITEM_HEIGHT);
}

void ChannelsItemDelegate::paint( QPainter* painter,
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

void ChannelsItemDelegate::paintAggregate(QPainter* painter,
                                          const QStyleOptionViewItem& option,
                                          const QModelIndex& index) const {
    painter->save();

    painter->translate(option.rect.topLeft());
    const QRect line(0, 0, option.rect.width(), option.rect.height());

    static const QPixmap thumbnail = QPixmap(":/images/channels.png");

    QString name = tr("All Videos");

    drawItem(painter, line, thumbnail, name);

    painter->restore();
}

void ChannelsItemDelegate::paintUnwatched(QPainter* painter,
                                          const QStyleOptionViewItem& option,
                                          const QModelIndex& index) const {
    painter->save();

    painter->translate(option.rect.topLeft());
    const QRect line(0, 0, option.rect.width(), option.rect.height());

    static const QPixmap thumbnail = QPixmap(":/images/unwatched.png");

    QString name = tr("Unwatched Videos");

    drawItem(painter, line, thumbnail, name);

    int notifyCount = ChannelAggregator::instance()->getUnwatchedCount();
    QString notifyText = QString::number(notifyCount);
    if (notifyCount > 0) paintBadge(painter, line, notifyText);

    painter->restore();
}

void ChannelsItemDelegate::paintChannel(QPainter* painter,
                                        const QStyleOptionViewItem& option,
                                        const QModelIndex& index) const {
    const QVariant dataObject = index.data(ChannelModel::DataObjectRole);
    const YTUserPointer channelPointer = dataObject.value<YTUserPointer>();
    YTUser *user = channelPointer.data();
    if (!user) return;

    painter->save();

    painter->translate(option.rect.topLeft());
    const QRect line(0, 0, option.rect.width(), option.rect.height());

    // const bool isActive = index.data( ActiveItemRole ).toBool();
    // const bool isHovered = index.data(ChannelsModel::HoveredItemRole ).toBool();
    // const bool isSelected = option.state & QStyle::State_Selected;

    QPixmap thumbnail = user->getThumbnail();
    if (thumbnail.isNull()) {
        user->loadThumbnail();
        painter->restore();
        return;
    }

    QString name = user->getDisplayName();
    drawItem(painter, line, thumbnail, name);

    int notifyCount = user->getNotifyCount();
    if (notifyCount > 0)
        paintBadge(painter, line, QString::number(notifyCount));

    painter->restore();
}

void ChannelsItemDelegate::drawItem(QPainter *painter,
                                    const QRect &line,
                                    const QPixmap &thumbnail,
                                    const QString &name) const {
    painter->drawPixmap((line.width() - THUMB_WIDTH) / 2, 8, thumbnail);

    QRect nameBox = line;
    nameBox.adjust(0, 0, 0, -THUMB_HEIGHT - 16);
    nameBox.translate(0, line.height() - nameBox.height());
    bool tooBig = false;

#ifdef APP_MAC_NO
    QFont f = painter->font();
    f.setFamily("Helvetica");
    painter->setFont(f);
#endif

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

void ChannelsItemDelegate::paintBadge(QPainter *painter,
                                              const QRect &line,
                                              const QString &text) const {
    const int topLeft = (line.width() + THUMB_WIDTH) / 2;
    painter->save();
    painter->translate(topLeft, 0);
    PainterUtils::paintBadge(painter, text, true);
    painter->restore();
}
