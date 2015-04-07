#include "channelwidget.h"
#include "videosource.h"
#include "ytuser.h"
#include "fontutils.h"

ChannelWidget::ChannelWidget(VideoSource *videoSource, YTUser *user, QWidget *parent) :
    GridWidget(parent) {
    this->user = user;
    this->videoSource = videoSource;

    setMinimumSize(132, 176);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    connect(user, SIGNAL(infoLoaded()), SLOT(gotUserInfo()));
    // user->loadfromAPI();

    connect(this, SIGNAL(activated()), SLOT(activate()));
}

void ChannelWidget::activate() {
    // YTUser.checked(user->getUserId());
    emit activated(videoSource);
}

void ChannelWidget::gotUserInfo() {
    connect(user, SIGNAL(thumbnailLoaded(QByteArray)), SLOT(gotUserThumbnail(QByteArray)));
    // user->loadThumbnail();
    update();
}

void ChannelWidget::paintEvent(QPaintEvent *) {
    if (thumbnail.isNull()) return;

    const int w = width();
    const int h = height();

    QPainter p(this);
    p.drawPixmap((w - thumbnail.width()) / 2, 0, thumbnail);
    //(h - thumbnail.height()) / 2

    QRect nameBox = rect();
    nameBox.adjust(0, 0, 0, -thumbnail.height() - 10);
    nameBox.translate(0, h - nameBox.height());

    QString name = user->getDisplayName();
    bool tooBig = false;
    p.save();
    /*
    QFont f = font();
    f.setFamily("Helvetica");
    p.setFont(f);
    */
    QRect textBox = p.boundingRect(nameBox, Qt::AlignTop | Qt::AlignHCenter | Qt::TextWordWrap, name);
    if (textBox.height() > nameBox.height()) {
        p.setFont(font());
        textBox = p.boundingRect(nameBox, Qt::AlignTop | Qt::AlignHCenter | Qt::TextWordWrap, name);
        if (textBox.height() > nameBox.height()) {
            p.setClipRect(nameBox);
            tooBig = true;
        }
    }
    p.setPen(Qt::black);
    if (tooBig)
        p.drawText(nameBox, Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap, name);
    else
        p.drawText(textBox, Qt::AlignCenter | Qt::TextWordWrap, name);
    p.restore();

    if (hasFocus()) {
        p.save();
        QPen pen;
        pen.setBrush(palette().highlight());
        pen.setWidth(2);
        p.setPen(pen);
        p.drawRect(rect());
        p.restore();
    }
}

void ChannelWidget::gotUserThumbnail(QByteArray bytes) {
    thumbnail.loadFromData(bytes);
    if (thumbnail.width() > 88)
        thumbnail = thumbnail.scaledToWidth(88, Qt::SmoothTransformation);
    update();
}
