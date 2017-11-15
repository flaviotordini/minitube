#include "toolbarmenu.h"
#include "mainwindow.h"
#include "sharetoolbar.h"

ToolbarMenu::ToolbarMenu(QWidget *parent) : QMenu(parent) {
    MainWindow *w = MainWindow::instance();
    addAction(w->getAction("stopafterthis"));
    addSeparator();
#ifdef APP_SNAPSHOT
    addAction(w->getAction("snapshot"));
#endif
    addAction(w->getAction("findVideoParts"));
    addSeparator();
    addAction(w->getAction("webpage"));
    addAction(w->getAction("videolink"));
    addAction(w->getAction("open-in-browser"));
    addAction(w->getAction("download"));
    addSeparator();
    QWidgetAction *widgetAction = new QWidgetAction(this);
    ShareToolbar *shareToolbar = new ShareToolbar();
    connect(this, &ToolbarMenu::leftMarginChanged, shareToolbar, &ShareToolbar::setLeftMargin);
    widgetAction->setDefaultWidget(shareToolbar);
    addAction(widgetAction);
    addSeparator();
    addAction(w->getAction("compactView"));
    addAction(w->getAction("ontop"));
    addSeparator();
    addAction(w->getAction("clearRecentKeywords"));
#ifndef APP_MAC
    addSeparator();
    addAction(w->getAction("toggle-menu"));
#endif
    addSeparator();
    addMenu(w->getMenu("help"));
}

void ToolbarMenu::showEvent(QShowEvent *e) {
    Q_UNUSED(e);
    QAction *a = MainWindow::instance()->getAction("stopafterthis");
    QStyleOptionMenuItem option;
    initStyleOption(&option, a);
    int leftMargin = option.maxIconWidth;
    emit leftMarginChanged(leftMargin);
}
