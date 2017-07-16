#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QtWidgets>

class ClickableLabel : public QLabel {

    Q_OBJECT

public:
    explicit ClickableLabel(QWidget *parent = 0);

signals:
    void clicked();

protected:
    void mouseReleaseEvent(QMouseEvent *e);

};

#endif // CLICKABLELABEL_H
