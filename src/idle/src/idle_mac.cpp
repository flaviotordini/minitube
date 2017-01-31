#include "idle.h"

#include <IOKit/pwr_mgt/IOPMLib.h>

namespace {

IOPMAssertionID displayAssertionID = 0;
IOReturn displayRes = 0;

IOPMAssertionID systemAssertionID = 0;
IOReturn systemRes = 0;

}

bool Idle::preventDisplaySleep(const QString &reason) {
    displayRes = IOPMAssertionCreateWithName(kIOPMAssertionTypePreventUserIdleDisplaySleep,
                                        kIOPMAssertionLevelOn, reason.toCFString(), &displayAssertionID);
    return displayRes == kIOReturnSuccess;
}

bool Idle::allowDisplaySleep() {
    displayRes = IOPMAssertionRelease(displayAssertionID);
    return displayRes == kIOReturnSuccess;
}

QString Idle::displayErrorMessage() {
    return QString();
    // return QString::fromUtf8(IOService::stringFromReturn(displayRes));
}

bool Idle::preventSystemSleep(const QString &reason) {
    systemRes = IOPMAssertionCreateWithName(kIOPMAssertionTypePreventUserIdleSystemSleep,
                                        kIOPMAssertionLevelOn, reason.toCFString(), &systemAssertionID);
    return systemRes == kIOReturnSuccess;
}

bool Idle::allowSystemSleep() {
    systemRes = IOPMAssertionRelease(systemAssertionID);
    return systemRes == kIOReturnSuccess;
}

QString Idle::systemErrorMessage() {
    return QString();
    // return QString::fromUtf8(IOService::stringFromReturn(systemRes));
}
