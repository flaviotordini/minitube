#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QtGui>
#include <phonon>

class VideoWidget : public Phonon::VideoWidget {

    Q_OBJECT

public:
    VideoWidget(QWidget *parent);

signals:
    void doubleClicked();
    void rightClicked();

protected:
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);

};

#endif // VIDEOWIDGET_H
