#ifndef SIDEBARWIDGET_H
#define SIDEBARWIDGET_H

#include <QtGui>

class RefineSearchButton;
class RefineSearchWidget;

class SidebarWidget : public QWidget {

    Q_OBJECT

public:
    SidebarWidget(QWidget *parent = 0);
    void setPlaylist(QListView *playlist);
    void showPlaylist();
    RefineSearchWidget* getRefineSearchWidget() { return refineSearchWidget; }

public slots:
    void showRefineSearchWidget();
    void hideRefineSearchWidget();
    void toggleRefineSearch(bool show = false);

protected:
    void resizeEvent(QResizeEvent *);
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    void mouseMoveEvent(QMouseEvent *event);
    bool eventFilter(QObject *, QEvent *);

private:
    void showRefineSearchButton();
    void setup();
    void handleMouseMove();

    QStackedWidget *stackedWidget;
    RefineSearchButton *refineSearchButton;
    QListView *playlist;
    RefineSearchWidget *refineSearchWidget;
    QTimer *mouseTimer;
    
};

#endif // SIDEBARWIDGET_H
