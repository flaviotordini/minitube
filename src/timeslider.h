#ifndef TIMESLIDER_H
#define TIMESLIDER_H

#include "seekslider.h"

class Media;


class TimeSlider : public SeekSlider
{
    Q_OBJECT

public:
    TimeSlider(QWidget *parent = nullptr);

    void setMedia(Media* newMedia);

protected:
    bool event(QEvent *event) override;

private:
    Media* media;

};

#endif // TIMESLIDER_H
