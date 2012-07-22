#include "sidebarwidget.h"
#include "refinesearchbutton.h"
#include "refinesearchwidget.h"
#ifndef Q_WS_X11
#include "extra.h"
#endif

namespace The {
QMap<QString, QAction*>* globalActions();
}

SidebarWidget::SidebarWidget(QWidget *parent) :
    QWidget(parent) {
    playlist = 0;

    QBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);

    stackedWidget = new QStackedWidget(this);
    layout->addWidget(stackedWidget);

    setup();
}

void SidebarWidget::setup() {
    static bool isSetup = false;
    if (isSetup) return;
    isSetup = true;

    refineSearchButton = new RefineSearchButton(this);
    refineSearchButton->setStatusTip(tr("Refine Search")
                                     + " (" + QKeySequence(Qt::CTRL + Qt::Key_R).toString(QKeySequence::NativeText) + ")");
    refineSearchButton->hide();
    connect(refineSearchButton, SIGNAL(clicked()), SLOT(showRefineSearchWidget()));

    refineSearchWidget = new RefineSearchWidget(this);
    connect(refineSearchWidget, SIGNAL(done()), SLOT(hideRefineSearchWidget()));
    stackedWidget->addWidget(refineSearchWidget);

    setMouseTracking(true);
    mouseTimer = new QTimer(this);
    mouseTimer->setInterval(5000);
    mouseTimer->setSingleShot(true);
    connect(mouseTimer, SIGNAL(timeout()), refineSearchButton, SLOT(hide()));
}

void SidebarWidget::setPlaylist(QListView *playlist) {
    this->playlist = playlist;
    playlist->installEventFilter(this);
    stackedWidget->addWidget(playlist);
}

void SidebarWidget::showPlaylist() {
    setup();
    stackedWidget->setCurrentWidget(playlist);
}

void SidebarWidget::showRefineSearchWidget() {
    refineSearchWidget->setDirty(false);
    stackedWidget->setCurrentWidget(refineSearchWidget);
    refineSearchWidget->setFocus();
#ifndef Q_WS_X11
    Extra::fadeInWidget(playlist, refineSearchWidget);
#endif
    refineSearchButton->hide();
    The::globalActions()->value("refine-search")->setChecked(true);
}

void SidebarWidget::hideRefineSearchWidget() {
    stackedWidget->setCurrentWidget(playlist);
    playlist->setFocus();
#ifndef Q_WS_X11
    Extra::fadeInWidget(refineSearchWidget, playlist);
#endif
    The::globalActions()->value("refine-search")->setChecked(false);
}

void SidebarWidget::toggleRefineSearch(bool show) {
    if (show) showRefineSearchWidget();
    else hideRefineSearchWidget();
}

void SidebarWidget::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    refineSearchButton->move(
                playlist->viewport()->width() - refineSearchButton->minimumWidth(),
                height() - refineSearchButton->minimumHeight());
}

void SidebarWidget::enterEvent(QEvent *) {
    if (stackedWidget->currentWidget() != refineSearchWidget)
        showRefineSearchButton();
}

void SidebarWidget::leaveEvent(QEvent *) {
    refineSearchButton->hide();
}

void SidebarWidget::mouseMoveEvent(QMouseEvent *event) {
    QWidget::mouseMoveEvent(event);
    handleMouseMove();
}

bool SidebarWidget::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::MouseMove) handleMouseMove();
    return QWidget::eventFilter(obj, event);
}

void SidebarWidget::handleMouseMove() {
    if (stackedWidget->currentWidget() != refineSearchWidget) {
        showRefineSearchButton();
        mouseTimer->start();
    }
}

void SidebarWidget::showRefineSearchButton() {
    refineSearchButton->move(
                playlist->viewport()->width() - refineSearchButton->minimumWidth(),
                height() - refineSearchButton->minimumHeight());
    refineSearchButton->show();
}
