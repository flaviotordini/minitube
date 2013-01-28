#ifndef HOMEVIEW_H
#define HOMEVIEW_H

#include <QtGui>
#include "view.h"

class SegmentedControl;
class SearchView;
class StandardFeedsView;
class UserView;

class HomeView : public QWidget, public View  {

    Q_OBJECT

public:
    HomeView(QWidget *parent = 0);
    void appear();
    void disappear();
    QHash<QString, QVariant> metadata() {
        QHash<QString, QVariant> metadata;
        metadata.insert("description", tr("Make yourself comfortable"));
        return metadata;
    }
    void showWidget(QWidget *widget);
    SearchView* getSearchView() { return searchView; }
    StandardFeedsView* getStandardFeedsView() { return standardFeedsView; }

public slots:
    void showSearch();
    void showStandardFeeds();
    void showUser();

private:
    void setupBar();
    SegmentedControl *bar;
    QStackedWidget *stackedWidget;

    SearchView *searchView;
    StandardFeedsView *standardFeedsView;
    UserView* userView;

};

#endif // HOMEVIEW_H
