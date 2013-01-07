#ifndef CATEGORIESVIEW_H
#define CATEGORIESVIEW_H

#include <QtGui>
#include "view.h"

class VideoSource;
class YTCategory;
class YTStandardFeed;

class StandardFeedsView : public QWidget, public View {

    Q_OBJECT

public:
    StandardFeedsView(QWidget *parent = 0);

signals:
    void activated(VideoSource *standardFeed);

public slots:
    void appear();
    void disappear();
    void load();
    
private slots:
    void layoutCategories(const QList<YTCategory> &categories);
    void selectWorldwideRegion();
    void selectLocalRegion();

private:
    void addVideoSourceWidget(VideoSource *videoSource);
    QList<YTStandardFeed*> getMainFeeds();
    YTStandardFeed* buildStardardFeed(QString feedId, QString label);
    QGridLayout *layout;
    
};

#endif // CATEGORIESVIEW_H
