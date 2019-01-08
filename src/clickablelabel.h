#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QtWidgets>

class ClickableLabel : public QLabel {
    Q_OBJECT

public:
    explicit ClickableLabel(QWidget *parent = nullptr);
    explicit ClickableLabel(const QString &text, QWidget *parent = nullptr);

signals:
    void clicked();
    void hovered(bool value);

protected:
    void mouseReleaseEvent(QMouseEvent *e);

    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
};

#endif // CLICKABLELABEL_H
