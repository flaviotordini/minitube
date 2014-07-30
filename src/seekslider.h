#ifndef SEEKSLIDER_H
#define SEEKSLIDER_H

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

class SeekSlider : public QSlider {

    Q_OBJECT

public:
    SeekSlider(QWidget *parent = 0);
    
};

#endif // SEEKSLIDER_H
