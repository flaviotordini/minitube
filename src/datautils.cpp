#include "datautils.h"

QString DataUtils::stringToFilename(const QString &s) {
    QString f = s;
    f.replace('(', '[');
    f.replace(')', ']');
    f.replace('/', ' ');
    f.replace('\\', ' ');
    f.replace('<', ' ');
    f.replace('>', ' ');
    f.replace(':', ' ');
    f.replace('"', ' ');
    f.replace('|', ' ');
    f.replace('?', ' ');
    f.replace('*', ' ');
    f = f.simplified();

    if (!f.isEmpty() && f.at(0) == '.')
        f = f.mid(1).trimmed();

    return f;
}
