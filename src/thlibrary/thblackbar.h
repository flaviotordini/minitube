#ifndef _THBLACKBAR_H_
#define _THBLACKBAR_H_

#include <QWidget>
#include <QAction>

class THBlackBar : public QWidget {
    
    Q_OBJECT
    
        public:
    THBlackBar (QWidget *parent = 0);
    ~THBlackBar();
    
        public:
    QAction *addAction (QAction *action);
    QAction *addAction (const QString& text);
    void setCheckedAction(int index);
    
    QSize minimumSizeHint (void) const;
    
        protected:
    void paintEvent (QPaintEvent *event);
    
    void mouseMoveEvent (QMouseEvent *event);
    void mousePressEvent (QMouseEvent *event);
    void leaveEvent(QEvent *event);
    
        private:
    void drawUnselectedButton (	QPainter *painter,
                                const QRect& rect,
                                const QAction *action);
    void drawSelectedButton (	QPainter *painter,
                                const QRect& rect,
                                const QAction *action);
    void drawButton (	QPainter *painter,
                        const QRect& rect,
                        const QAction *action);
    void drawButton (	QPainter *painter,
                        const QRect& rect,
                        const QLinearGradient& gradient,
                        const QColor& color,
                        const QAction *action);
    
    QAction *hoveredAction (const QPoint& pos) const;
    int calculateButtonWidth (void) const;		
    
        private:
    class Private;
    Private *d;
};

#endif /* !_THBLACKBAR_H_ */

