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
    void hideSuggestions();

public slots:
    void showRefineSearchWidget();
    void hideRefineSearchWidget();
    void toggleRefineSearch(bool show = false);
    void showSuggestions(const QStringList &suggestions);

signals:
    void suggestionAccepted(QString);

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
    QLabel *messageLabel;
    
};

#endif // SIDEBARWIDGET_H
