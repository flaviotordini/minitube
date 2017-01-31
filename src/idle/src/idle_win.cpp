#include "idle.h"

#include "windows.h"

namespace {
EXECUTION_STATE executionState;
}

bool Idle::preventDisplaySleep(const QString &reason) {
    executionState = SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED);
    return true;
}

bool Idle::allowDisplaySleep() {
    SetThreadExecutionState(ES_CONTINUOUS | executionState);
    return true;
}

QString Idle::displayErrorMessage() {
    return QString();
}

bool Idle::preventSystemSleep(const QString &reason) {
    executionState = SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED);
    return true;
}

bool Idle::allowSystemSleep() {
    SetThreadExecutionState(ES_CONTINUOUS | executionState);
    return true;
}

QString Idle::systemErrorMessage() {
    return QString();
}
