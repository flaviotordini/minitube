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
#include "videosourcewidget.h"
#include "ytcategories.h"
#include "ytstandardfeed.h"
#include "ytregions.h"
#include "mainwindow.h"
#include "painterutils.h"

StandardFeedsView::StandardFeedsView(QWidget *parent) : View(parent),
    layout(0) {
    QPalette p = palette();
    p.setBrush(QPalette::Window, Qt::black);
    setPalette(p);
    setAutoFillBackground(true);

    connect(MainWindow::instance()->getActionMap().value("worldwide-region"), SIGNAL(triggered()),
            SLOT(selectWorldwideRegion()));

    connect(MainWindow::instance()->getActionMap().value("local-region"), SIGNAL(triggered()),
            SLOT(selectLocalRegion()));

    /*
    QAction *regionAction = MainWindow::instance()->getRegionAction();
    connect(regionAction, SIGNAL(changed()), SLOT(load()));
    */
}

void StandardFeedsView::load() {
    setUpdatesEnabled(false);
    YTCategories *youTubeCategories = new YTCategories(this);
    connect(youTubeCategories, SIGNAL(categoriesLoaded(const QList<YTCategory> &)),
            SLOT(layoutCategories(const QList<YTCategory> &)));
    youTubeCategories->loadCategories();

    if (layout) {
        while (QLayoutItem *item = layout->takeAt(0)) {
            delete item->widget();
            delete item;
        }
        delete layout;
    }

    layout = new QGridLayout(this);
    layout->setMargin(0);
    layout->setSpacing(1);

    QList<YTStandardFeed*> feeds = getMainFeeds();
    foreach(YTStandardFeed *feed, feeds)
        addVideoSourceWidget(feed);

    YTRegion region = YTRegions::currentRegion();
    QAction *regionAction = MainWindow::instance()->getRegionAction();
    regionAction->setText(region.name);
    regionAction->setIcon(YTRegions::iconForRegionId(region.id));
}

void StandardFeedsView::layoutCategories(const QList<YTCategory> &categories) {
    QString regionId = YTRegions::currentRegionId();
    foreach(YTCategory category, categories) {
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
    connect(w, SIGNAL(activated(VideoSource*)),
            SIGNAL(activated(VideoSource*)));
    int i = layout->count();
    const int cols = 5;
    layout->addWidget(w, i / cols, i % cols);
}

QList<YTStandardFeed*> StandardFeedsView::getMainFeeds() {
    QList<YTStandardFeed*> feeds;

    feeds << buildStardardFeed("most_popular", tr("Most Popular"));
          // << buildStardardFeed("recently_featured", tr("Featured"))
          // << buildStardardFeed("most_shared", tr("Most Shared"))
          // << buildStardardFeed("most_discussed", tr("Most Discussed"))
          // << buildStardardFeed("top_rated", tr("Top Rated"))
          // << buildStardardFeed("most_popular", tr("All Time Popular"), "all_time");

    return feeds;
}

YTStandardFeed* StandardFeedsView::buildStardardFeed(const QString &feedId, const QString &label, QString time) {
    YTStandardFeed *feed = new YTStandardFeed(this);
    feed->setFeedId(feedId);
    feed->setLabel(label);
    if (!time.isEmpty()) feed->setTime(time);
    feed->setRegionId(YTRegions::currentRegionId());
    return feed;
}

void StandardFeedsView::appear() {
    setFocus();
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

