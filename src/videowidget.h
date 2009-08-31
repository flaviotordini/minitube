#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QtGui>
#include <phonon>
#include <QTimer>

class VideoWidget : public Phonon::VideoWidget {

    Q_OBJECT

public:
    VideoWidget(QWidget *parent);

protected:
    void mouseMoveEvent (QMouseEvent *event);

private slots:
    void hideMouse();

private:
    QTimer *mouseTimer;

};

#endif // VIDEOWIDGET_H
