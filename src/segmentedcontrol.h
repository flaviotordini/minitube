#ifndef SEGMENTEDCONTROL_H
#define SEGMENTEDCONTROL_H

#include <QtGui>

class SegmentedControl : public QWidget {

    Q_OBJECT

public:
    SegmentedControl(QWidget *parent = 0);
    ~SegmentedControl();
    QAction *addAction(QAction *action);
    bool setCheckedAction(int index);
    bool setCheckedAction(QAction *action);
    QSize minimumSizeHint(void) const;

signals:
    void checkedActionChanged(QAction & action);

protected:
    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void leaveEvent(QEvent *event);

private:
    void drawButton(QPainter *painter,
                    const QRect& rect,
                    const QAction *action);
    void drawUnselectedButton(QPainter *painter,
                              const QRect& rect,
                              const QAction *action);
    void drawSelectedButton(QPainter *painter,
                            const QRect& rect,
                            const QAction *action);
    void paintButton(QPainter *painter,
                    const QRect& rect,
                    const QAction *action);
    QAction *hoveredAction(const QPoint& pos) const;
    int calculateButtonWidth(void) const;

    class Private;
    Private *d;

};

#endif /* !SEGMENTEDCONTROL_H */
