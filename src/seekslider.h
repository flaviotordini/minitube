#ifndef SEEKSLIDER_H
#define SEEKSLIDER_H

#include <QtWidgets>

class SeekSlider : public QSlider {
    Q_OBJECT

public:
    SeekSlider(QWidget *parent = nullptr);
};

#endif // SEEKSLIDER_H
