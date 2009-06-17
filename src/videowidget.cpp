#include "videowidget.h"

VideoWidget::VideoWidget(QWidget *parent) : Phonon::VideoWidget(parent) {

}

void VideoWidget::mouseDoubleClickEvent(QMouseEvent *event) {
    switch(event->button()) {
             case Qt::LeftButton:
        emit doubleClicked();
        break;
             case Qt::RightButton:

        break;
    }
}

void VideoWidget::mousePressEvent(QMouseEvent *event) {
    switch(event->button()) {
             case Qt::LeftButton:

        break;
             case Qt::RightButton:
        emit rightClicked();
        break;
    }
}
