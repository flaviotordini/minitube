#include "globalshortcutbackend.h"
#include "globalshortcuts.h"

GlobalShortcutBackend::GlobalShortcutBackend(GlobalShortcuts *parent)
    : QObject(parent),
    manager_(parent),
    active_(false) { }

bool GlobalShortcutBackend::Register() {
    bool ret = DoRegister();
    if (ret)
        active_ = true;
    return ret;
}

void GlobalShortcutBackend::Unregister() {
    DoUnregister();
    active_ = false;
}

void GlobalShortcutBackend::Reregister() {
    Unregister();
    Register();
}
