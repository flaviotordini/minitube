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

#include "standardfeedsview.h"
#include "mainwindow.h"
#include "painterutils.h"
#include "videosourcewidget.h"
#include "ytcategories.h"
#include "ytregions.h"
#include "ytstandardfeed.h"

StandardFeedsView::StandardFeedsView(QWidget *parent) : View(parent), layout(0) {
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);

    connect(MainWindow::instance()->getAction("worldwideRegion"), SIGNAL(triggered()),
            SLOT(selectWorldwideRegion()));

    connect(MainWindow::instance()->getAction("localRegion"), SIGNAL(triggered()),
            SLOT(selectLocalRegion()));
}

void StandardFeedsView::load() {
    setUpdatesEnabled(false);
    YTCategories *youTubeCategories = new YTCategories(this);
    connect(youTubeCategories, SIGNAL(categoriesLoaded(const QVector<YTCategory> &)),
            SLOT(layoutCategories(const QVector<YTCategory> &)));
    youTubeCategories->loadCategories();

    resetLayout();

    addVideoSourceWidget(buildStandardFeed("most_popular", tr("Most Popular")));

    YTRegion region = YTRegions::currentRegion();
    QAction *regionAction = MainWindow::instance()->getRegionAction();
    regionAction->setText(region.name);
    regionAction->setIcon(YTRegions::iconForRegionId(region.id));
}

void StandardFeedsView::layoutCategories(const QVector<YTCategory> &categories) {
    QString regionId = YTRegions::currentRegionId();
    for (const YTCategory &category : categories) {
        // assign a parent to this VideoSource  so it won't be deleted by MediaView
        YTStandardFeed *feed = new YTStandardFeed(this);
        feed->setCategory(category.term);
        feed->setLabel(category.label);
        feed->setRegionId(regionId);
        feed->setFeedId("most_popular");
        addVideoSourceWidget(feed);
    }
    if (categories.size() > 1) setUpdatesEnabled(true);
}

void StandardFeedsView::addVideoSourceWidget(VideoSource *videoSource) {
    VideoSourceWidget *w = new VideoSourceWidget(videoSource);
    connect(w, SIGNAL(activated(VideoSource *)), SIGNAL(activated(VideoSource *)));
    connect(w, SIGNAL(unavailable(VideoSourceWidget *)),
            SLOT(removeVideoSourceWidget(VideoSourceWidget *)));
    int i = layout->count();
    const int cols = 5;
    layout->addWidget(w, i / cols, i % cols);
}

void StandardFeedsView::removeVideoSourceWidget(VideoSourceWidget *videoSourceWidget) {
    qDebug() << videoSourceWidget->getVideoSource()->getName();
    layout->removeWidget(videoSourceWidget);
    videoSourceWidget->deleteLater();

    const int layoutCount = layout->count();
    QVector<QLayoutItem *> items;
    items.reserve(layoutCount);

    for (int i = layoutCount - 1; i >= 0; i--) {
        QLayoutItem *item = layout->takeAt(i);
        if (item && item->widget()) items.append(item);
    }

    const int itemCount = items.size();
    const int cols = 4; // itemCount / 3;
    for (int i = itemCount - 1; i >= 0; i--) {
        QLayoutItem *item = items.at(i);
        int index = itemCount - 1 - i;
        layout->addItem(item, index / cols, index % cols);
    }
}

void StandardFeedsView::resetLayout() {
    if (layout) {
        while (QLayoutItem *item = layout->takeAt(0)) {
            delete item->widget();
            delete item;
        }
        delete layout;
    }

    layout = new QGridLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
}

YTStandardFeed *
StandardFeedsView::buildStandardFeed(const QString &feedId, const QString &label, QString time) {
    YTStandardFeed *feed = new YTStandardFeed(this);
    feed->setFeedId(feedId);
    feed->setLabel(label);
    if (!time.isEmpty()) feed->setTime(time);
    feed->setRegionId(YTRegions::currentRegionId());
    return feed;
}

void StandardFeedsView::appear() {
    if (!layout) {
        update();
        qApp->processEvents();
        load();
    }
    QAction *regionAction = MainWindow::instance()->getRegionAction();
    MainWindow::instance()->showActionInStatusBar(regionAction, true);
}

void StandardFeedsView::disappear() {
    QAction *regionAction = MainWindow::instance()->getRegionAction();
    MainWindow::instance()->showActionInStatusBar(regionAction, false);
}

void StandardFeedsView::selectWorldwideRegion() {
    YTRegions::setRegion(YTRegions::worldwideRegion().id);
    load();
}

void StandardFeedsView::selectLocalRegion() {
    YTRegions::setRegion(YTRegions::localRegion().id);
    load();
}

void StandardFeedsView::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);
}
