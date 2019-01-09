#include "sharetoolbar.h"
#include "iconutils.h"
#include "mainwindow.h"

ShareToolbar::ShareToolbar(QWidget *parent) : QToolBar(parent) {
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    setIconSize(QSize(18, 18));
    MainWindow *w = MainWindow::instance();
    addAction(w->getAction("pagelink"));
    addAction(w->getAction("twitter"));
    addAction(w->getAction("facebook"));
    addAction(w->getAction("email"));
}

void ShareToolbar::setLeftMargin(int value) {
    setStyleSheet("QToolButton {border:0;margin-left:" + QString::number(value) + "px}");
    disconnect(sender(), nullptr, this, nullptr);
}
