#include "timeslider.h"
#include "media.h"
#include "helper.h"

#include <QHelpEvent>

TimeSlider::TimeSlider(QWidget *parent) : SeekSlider(parent), media(nullptr) {
    setMouseTracking( true );
    setMaximum(1000);
}

void TimeSlider::setMedia(Media* newMedia)
{
    media = newMedia;
}

bool TimeSlider::event(QEvent *event)
{
    if (isEnabled() && event->type() == QEvent::ToolTip && nullptr != media) {
        QHelpEvent * help_event = static_cast<QHelpEvent *>(event);
        const qint64 duration = media->duration();

        if (duration > 0) {
            const qreal percentage = (qreal) help_event->x() / (qreal) width();
            const qint64 time = (percentage * duration);

            if (time > 0 && time <= duration) {
                QToolTip::showText(help_event->globalPos(), Helper::formatTime(time), this);
            } else {
                QToolTip::hideText();
                event->ignore();
            }
        }
        else {
            QToolTip::hideText();
            event->ignore();
        }
        return true;
    }
    return SeekSlider::event(event);
}
