#include "gnomeglobalshortcutbackend.h"
#include "globalshortcuts.h"

#include <QAction>
#include <QtDebug>

#ifdef QT_DBUS_LIB
#  include <QtDBus>
#endif

const char* GnomeGlobalShortcutBackend::kGsdService = "org.gnome.SettingsDaemon";
const char* GnomeGlobalShortcutBackend::kGsdPath = "/org/gnome/SettingsDaemon/MediaKeys";
const char* GnomeGlobalShortcutBackend::kGsdInterface = "org.gnome.SettingsDaemon.MediaKeys";

GnomeGlobalShortcutBackend::GnomeGlobalShortcutBackend(GlobalShortcuts* parent)
    : GlobalShortcutBackend(parent),
    interface_(NULL) { }

bool GnomeGlobalShortcutBackend::IsGsdAvailable() {
#ifdef QT_DBUS_LIB
    return QDBusConnection::sessionBus().interface()->isServiceRegistered(
            GnomeGlobalShortcutBackend::kGsdService);
#else // QT_DBUS_LIB
    return false;
#endif
}

bool GnomeGlobalShortcutBackend::DoRegister() {
    // qDebug() << __PRETTY_FUNCTION__;
#ifdef QT_DBUS_LIB
    // Check if the GSD service is available
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(kGsdService))
        return false;

    if (!interface_) {
        interface_ = new QDBusInterface(
                kGsdService, kGsdPath, kGsdInterface, QDBusConnection::sessionBus(), this);
    }

    connect(interface_, SIGNAL(MediaPlayerKeyPressed(QString,QString)),
            this, SLOT(GnomeMediaKeyPressed(QString,QString)));

    return true;
#else // QT_DBUS_LIB
    return false;
#endif
}

void GnomeGlobalShortcutBackend::DoUnregister() {

#ifdef QT_DBUS_LIB
    // Check if the GSD service is available
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(kGsdService))
        return;
    if (!interface_)
        return;

    disconnect(interface_, SIGNAL(MediaPlayerKeyPressed(QString,QString)),
               this, SLOT(GnomeMediaKeyPressed(QString,QString)));
#endif
}

void GnomeGlobalShortcutBackend::GnomeMediaKeyPressed(const QString&, const QString& key) {
    if (key == "Play")     manager_->shortcuts()["play_pause"].action->trigger();
    if (key == "Stop")     manager_->shortcuts()["stop"].action->trigger();
    if (key == "Next")     manager_->shortcuts()["next_track"].action->trigger();
    if (key == "Previous") manager_->shortcuts()["prev_track"].action->trigger();
}
