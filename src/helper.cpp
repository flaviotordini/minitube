#include "helper.h"

QString Helper::formatTime(qint64 duration) {
    duration /= 1000;
    QString res;
    int seconds = (int)(duration % 60);
    duration /= 60;
    int minutes = (int)(duration % 60);
    duration /= 60;
    int hours = (int)(duration % 24);
    if (hours == 0) return res.sprintf("%02d:%02d", minutes, seconds);
    return res.sprintf("%02d:%02d:%02d", hours, minutes, seconds);
}
