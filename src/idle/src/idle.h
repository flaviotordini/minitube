#ifndef IDLE_H
#define IDLE_H

#include <QString>

class Idle {

public:
    static bool preventDisplaySleep(const QString &reason);
    static bool allowDisplaySleep();
    static QString displayErrorMessage();

    static bool preventSystemSleep(const QString &reason);
    static bool allowSystemSleep();
    static QString systemErrorMessage();

};

#endif // IDLE_H
