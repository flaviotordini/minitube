#ifndef SHAREMENUTOOLBAR_H
#define SHAREMENUTOOLBAR_H

#include <QtWidgets>

class ShareToolbar : public QToolBar {
    Q_OBJECT

public:
    ShareToolbar(QWidget *parent = nullptr);

public slots:
    void setLeftMargin(int value);
};

#endif // SHAREMENUTOOLBAR_H
