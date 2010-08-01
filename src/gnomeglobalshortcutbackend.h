#ifndef GNOMEGLOBALSHORTCUTBACKEND_H
#define GNOMEGLOBALSHORTCUTBACKEND_H

#include "globalshortcutbackend.h"

class QDBusInterface;

class GnomeGlobalShortcutBackend : public GlobalShortcutBackend {
    Q_OBJECT

public:
    GnomeGlobalShortcutBackend(GlobalShortcuts* parent);
    static bool IsGsdAvailable();
    static const char* kGsdService;
    static const char* kGsdPath;
    static const char* kGsdInterface;

protected:
    bool DoRegister();
    void DoUnregister();

private slots:
    void GnomeMediaKeyPressed(const QString& application, const QString& key);

private:
    QDBusInterface* interface_;
};

#endif // GNOMEGLOBALSHORTCUTBACKEND_H
