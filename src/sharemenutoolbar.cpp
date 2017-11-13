#include "sharemenutoolbar.h"
#include "iconutils.h"
#include "mainwindow.h"

ShareMenuToolbar::ShareMenuToolbar(QWidget *parent) : QToolBar(parent) {
    setStyleSheet("border:0;margin-left:18px");
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    setIconSize(QSize(18, 18));
    MainWindow *w = MainWindow::instance();
    addAction(w->getAction("videolink"));
    addAction(w->getAction("twitter"));
    addAction(w->getAction("facebook"));
    addAction(w->getAction("email"));
}
