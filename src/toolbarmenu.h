#ifndef TOOLBARMENU_H
#define TOOLBARMENU_H

#include <QtWidgets>

class ToolbarMenu : public QMenu {
    Q_OBJECT

public:
    ToolbarMenu(QWidget *parent = 0);

signals:
    void leftMarginChanged(int value);

protected:
    void showEvent(QShowEvent *e);
};

#endif // TOOLBARMENU_H
