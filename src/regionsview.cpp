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

#include "regionsview.h"
#include "mainwindow.h"
#include "ytregions.h"

RegionsView::RegionsView(QWidget *parent) : View(parent) {
    QBoxLayout *l = new QVBoxLayout(this);
    l->setMargin(30);
    l->setSpacing(30);

    layout = new QGridLayout();
    layout->setMargin(0);
    layout->setSpacing(0);
    l->addLayout(layout);

    addRegion(YTRegions::worldwideRegion());
    foreach (YTRegion region, YTRegions::list())
        addRegion(region);

    doneButton = new QPushButton(tr("Done"));
    doneButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    doneButton->setDefault(true);
#ifndef APP_MAC
    doneButton->setProperty("custom", true);
    doneButton->setProperty("important", true);
    doneButton->setProperty("big", true);
#endif
    connect(doneButton, SIGNAL(clicked()), MainWindow::instance(), SLOT(goBack()));
    l->addWidget(doneButton, 0, Qt::AlignHCenter | Qt::AlignTop);
}

void RegionsView::addRegion(const YTRegion &region) {
    QPushButton *button = new QPushButton(region.name);
    button->setProperty("regionId", region.id);
    button->setCheckable(true);
    button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    button->setFocusPolicy(Qt::StrongFocus);
    button->setIcon(YTRegions::iconForRegionId(region.id));
    connect(button, SIGNAL(clicked()), SLOT(buttonClicked()));
    const int i = layout->count();
    static const int rows = 10;
    layout->addWidget(button, i % rows, i / rows);
}

void RegionsView::appear() {
    doneButton->setFocus();

    QString currentRegionId = YTRegions::currentRegionId();
    for (int i = 0; i < layout->count(); i++) {
        QLayoutItem *item = layout->itemAt(i);
        QPushButton *b = static_cast<QPushButton *>(item->widget());
        QString regionId = b->property("regionId").toString();
        b->setChecked(currentRegionId == regionId);
    }
}

void RegionsView::paintEvent(QPaintEvent *e) {
    QWidget::paintEvent(e);
    QBrush brush;
    if (window()->isActiveWindow()) {
        brush = palette().base();
    } else {
        brush = palette().window();
    }
    QPainter painter(this);
    painter.fillRect(0, 0, width(), height(), brush);
    painter.end();
}

void RegionsView::buttonClicked() {
    QObject *o = sender();
    QString regionId = o->property("regionId").toString();
    YTRegions::setRegion(regionId);
    emit regionChanged();
    doneButton->click();

    // uncheck other buttons
    /*
    for (int i = 0; i < layout->count(); i++) {
        QLayoutItem *item = layout->itemAt(i);
        QPushButton *b = static_cast<QPushButton*>(item->widget());
        if (b != o && b->isChecked()) b->setChecked(false);
    }
    */
}
