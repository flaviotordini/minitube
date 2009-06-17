#ifndef FADERWIDGET_H
#define FADERWIDGET_H

#include <QtGui>

class FaderWidget : public QWidget {

    Q_OBJECT
    Q_PROPERTY(int fadeDuration READ fadeDuration WRITE setFadeDuration)

public:

    FaderWidget(QWidget *parent);

    int fadeDuration() const {
        return timeLine->duration();
    }
    void setFadeDuration(int milliseconds) {
        timeLine->setDuration(milliseconds);
    }
    void start(QPixmap frozenView);

protected:
    void paintEvent(QPaintEvent *event);

private:
    QTimeLine *timeLine;
    QPixmap frozenView;

};

#endif
