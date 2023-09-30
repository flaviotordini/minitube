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
#include "ytregions.h"
#ifdef YT_YT3
#include "ytcategories.h"
#include "ytstandardfeed.h"
#endif
#ifdef YT_IV
#include "ivvideolist.h"
#endif
#include "videoapi.h"

#include "ytjstrending.h"

StandardFeedsView::StandardFeedsView(QWidget *parent) : QWidget(parent), layout(0) {
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);

    connect(MainWindow::instance()->getAction("worldwideRegion"), SIGNAL(triggered()),
            SLOT(selectWorldwideRegion()));

    connect(MainWindow::instance()->getAction("localRegion"), SIGNAL(triggered()),
            SLOT(selectLocalRegion()));
}

void StandardFeedsView::load() {
    setUpdatesEnabled(false);
    resetLayout();

    YTRegion region = YTRegions::currentRegion();

    // TODO consolidate in YT
#ifdef YT_YT3
    auto buildStandardFeed = [this](const QString &feedId, const QString &label, QString time) {
        auto feed = new YTStandardFeed(this);
        feed->setFeedId(feedId);
        feed->setLabel(label);
        if (!time.isEmpty()) feed->setTime(time);
        feed->setRegionId(YTRegions::currentRegionId());
        return feed;
    };

    YTCategories *youTubeCategories = new YTCategories(this);
    connect(youTubeCategories, &YTCategories::categoriesLoaded, this, [](auto categories) {
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
    });
    youTubeCategories->loadCategories();
    addVideoSourceWidget(buildStandardFeed("most_popular", tr("Most Popular")));
#endif

    const QMap<QString, QString> pages = {{"default", tr("Trending")},
                                          {"music", tr("Music")},
                                          {"movies", tr("Movies")},
                                          {"gaming", tr("Gaming")}};
    auto i = pages.constBegin();
    while (i != pages.constEnd()) {
        addVideoSourceWidget(
                new YTJSTrending(i.value(), {{"page", i.key()}, {"geoLocation", region.id}}));
        ++i;
    }

#ifdef YT_IV
    QString regionParam = "region=" + region.id;
    addVideoSourceWidget(new IVVideoList("popular?" + regionParam, tr("Most Popular")));
    addVideoSourceWidget(new IVVideoList("trending?" + regionParam, tr("Trending")));
    addVideoSourceWidget(new IVVideoList("trending?type=music&" + regionParam, tr("Music")));
    addVideoSourceWidget(new IVVideoList("trending?type=news&" + regionParam, tr("News")));
    addVideoSourceWidget(new IVVideoList("trending?type=movies&" + regionParam, tr("Movies")));
    addVideoSourceWidget(new IVVideoList("trending?type=gaming&" + regionParam, tr("Gaming")));
#endif

    setUpdatesEnabled(true);

    QAction *regionAction = MainWindow::instance()->getRegionAction();
    regionAction->setText(region.name);
    regionAction->setIcon(YTRegions::iconForRegionId(region.id));

    loadNextPreview();
}

void StandardFeedsView::loadNextPreview(VideoSourceWidget *previous) {
    int index = 0;
    if (previous) {
        index = sourceWidgets.indexOf(previous);
        index++;
        if (index == sourceWidgets.length() || index < 0) return;
    }
    auto w = sourceWidgets.at(index);
    w->loadPreview()->finally([this, w] { loadNextPreview(w); });
}

void StandardFeedsView::addVideoSourceWidget(VideoSource *videoSource) {
    VideoSourceWidget *w = new VideoSourceWidget(videoSource);
    connect(w, SIGNAL(activated(VideoSource *)), SIGNAL(activated(VideoSource *)));
    connect(w, SIGNAL(unavailable(VideoSourceWidget *)),
            SLOT(removeVideoSourceWidget(VideoSourceWidget *)));
    int i = layout->count();
    const int cols = VideoAPI::impl() == VideoAPI::YT3 ? 5 : 2;
    layout->addWidget(w, i / cols, i % cols);

    connect(w, &QObject::destroyed, this, [this, w] { sourceWidgets.removeOne(w); });
    sourceWidgets << w;
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
    const int cols = 2; // itemCount / 3;
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
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
}

void StandardFeedsView::showEvent(QShowEvent *event) {
    if (!layout) {
        update();
        qApp->processEvents();
        load();
    }
    QAction *regionAction = MainWindow::instance()->getRegionAction();
    MainWindow::instance()->showActionsInStatusBar({regionAction}, true);
}

void StandardFeedsView::hideEvent(QHideEvent *event) {
    QAction *regionAction = MainWindow::instance()->getRegionAction();
    MainWindow::instance()->showActionsInStatusBar({regionAction}, false);
}

void StandardFeedsView::selectWorldwideRegion() {
    YTRegions::setRegion(YTRegions::defaultRegion().id);
    load();
}

void StandardFeedsView::selectLocalRegion() {
    YTRegions::setRegion(YTRegions::localRegion().id);
    load();
}

void StandardFeedsView::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);
}
