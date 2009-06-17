#include "spacer.h"

Spacer::Spacer(QWidget *parent, QWidget *child) : QWidget(parent) {
    QBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(child);
    setLayout(layout);
}
