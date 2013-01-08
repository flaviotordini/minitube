#include "utils.h"
#ifndef Q_WS_X11
#include "extra.h"
#endif

QIcon Utils::icon(const QString &name) {
#ifdef Q_WS_X11
    return QIcon::fromTheme(name);
#else
    return Extra::getIcon(name);
#endif
}
