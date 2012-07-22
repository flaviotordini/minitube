#include "refinesearchbutton.h"

static const int refineButtonSize = 48;

RefineSearchButton::RefineSearchButton(QWidget *parent) :
    QPushButton(parent) {

    hovered = false;

    setMinimumSize(refineButtonSize, refineButtonSize);
    setMaximumSize(refineButtonSize, refineButtonSize);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setStyleSheet(
                "background: red url(:/images/refine-search.png) no-repeat center;"
                "border: 0;"
                );
}

void RefineSearchButton::paintBackground() const {

}

void RefineSearchButton::paintEvent(QPaintEvent *event) {
    // QPushButton::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing, true);
    painter.setBrush(QColor(0,0,0, hovered ? 192 : 170));
    QPen pen(Qt::white);
    pen.setWidth(2);
    painter.setPen(pen);
    painter.drawEllipse(QPoint(width(), height()), width()-2, height()-2);

    QPixmap icon = QPixmap(":/images/refine-search.png");
    painter.drawPixmap(width() - icon.width() - 6, height() - icon.height() - 6,
                       icon.width(), icon.height(),
                       icon);
}

void RefineSearchButton::enterEvent(QEvent *) {
    hovered = true;
    update();
}

void RefineSearchButton::leaveEvent(QEvent *) {
    hovered = false;
    update();
}
