#include "utils.h"
#ifndef Q_WS_X11
#include "extra.h"
#endif

QIcon getIcon(const QString &name) {
#ifdef Q_WS_X11
    return QIcon::fromTheme(name);
#else
    return Extra::getIcon(name);
#endif
}

QIcon Utils::icon(const QString &name) {
#ifdef Q_WS_X11
    QString themeName = qApp->property("style").toString();
    if (themeName == QLatin1String("Ambiance"))
        return icon(QStringList() << name + "-symbolic" << name);
    else return getIcon(name);
#else
    return Extra::getIcon(name);
#endif
}

QIcon Utils::icon(const QStringList &names) {
    QIcon icon;
    foreach (QString name, names) {
        icon = getIcon(name);
        if (!icon.availableSizes().isEmpty()) break;
    }
    return icon;
}
