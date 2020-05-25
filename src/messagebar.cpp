#include "messagebar.h"
#include "iconutils.h"

MessageBar::MessageBar(QWidget *parent) : QWidget(parent) {
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    QBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(16);

    msgLabel = new QLabel();
    msgLabel->setOpenExternalLinks(true);
    layout->addWidget(msgLabel);

    QToolButton *closeToolButton = new QToolButton();
    closeToolButton->setIcon(IconUtils::icon("close"));
    connect(closeToolButton, &QToolButton::clicked, this, [this] {
        emit closed();
        hide();
    });
    layout->addWidget(closeToolButton);
}

void MessageBar::setMessage(const QString &message) {
    msgLabel->setText(message);
}

void MessageBar::paintEvent(QPaintEvent *e) {
    Q_UNUSED(e);
    QStyleOption o;
    o.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &o, &p, this);
}
