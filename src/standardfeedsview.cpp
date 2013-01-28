#include "standardfeedsview.h"
#include "videosourcewidget.h"
#include "ytcategories.h"
#include "ytstandardfeed.h"
#include "ytregions.h"
#include "mainwindow.h"

namespace The {
QHash<QString, QAction*>* globalActions();
}

static const int cols = 5;

StandardFeedsView::StandardFeedsView(QWidget *parent) : QWidget(parent),
    layout(0) {
    QPalette p = palette();
    p.setBrush(QPalette::Window, Qt::black);
    setPalette(p);
    setAutoFillBackground(true);

    connect(The::globalActions()->value("worldwide-region"), SIGNAL(triggered()),
            SLOT(selectWorldwideRegion()));

    connect(The::globalActions()->value("local-region"), SIGNAL(triggered()),
            SLOT(selectLocalRegion()));

    /*
    QAction *regionAction = MainWindow::instance()->getRegionAction();
    connect(regionAction, SIGNAL(changed()), SLOT(load()));
    */
}

void StandardFeedsView::load() {
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
    QToolButton *regionButton = MainWindow::instance()->getRegionButton();
    regionButton->setText(region.name);
    regionButton->setIcon(YTRegions::iconForRegionId(region.id));
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
}

void StandardFeedsView::addVideoSourceWidget(VideoSource *videoSource) {
    VideoSourceWidget *w = new VideoSourceWidget(videoSource);
    connect(w, SIGNAL(activated(VideoSource*)),
            SIGNAL(activated(VideoSource*)));
    int i = layout->count();
    layout->addWidget(w, i / cols, i % cols);
}

QList<YTStandardFeed*> StandardFeedsView::getMainFeeds() {
    QList<YTStandardFeed*> feeds;

    feeds << buildStardardFeed("most_popular", tr("Most Popular"))
          << buildStardardFeed("recently_featured", tr("Featured"))
          << buildStardardFeed("most_shared", tr("Most Shared"))
          << buildStardardFeed("most_discussed", tr("Most Discussed"))
          << buildStardardFeed("top_rated", tr("Top Rated"));

    return feeds;
}

YTStandardFeed* StandardFeedsView::buildStardardFeed(QString feedId, QString label) {
    YTStandardFeed *feed = new YTStandardFeed(this);
    feed->setFeedId(feedId);
    feed->setLabel(label);
    feed->setRegionId(YTRegions::currentRegionId());
    return feed;
}

void StandardFeedsView::appear() {
    setFocus();
    if (!layout) load();
    QAction *regionAction = MainWindow::instance()->getRegionAction();
    regionAction->setVisible(true);
}

void StandardFeedsView::disappear() {
    QAction *regionAction = MainWindow::instance()->getRegionAction();
    regionAction->setVisible(false);
}

void StandardFeedsView::selectWorldwideRegion() {
    YTRegions::setRegion(YTRegions::worldwideRegion().id);
    load();
}

void StandardFeedsView::selectLocalRegion() {
    YTRegions::setRegion(YTRegions::localRegion().id);
    load();
}


