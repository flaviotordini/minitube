#ifndef UTILS_H
#define UTILS_H

#include <QtGui>

class Utils {

public:
    static QIcon icon(const QString &name);
    static QIcon icon(const QStringList &names);

private:
    Utils() { }

};

#endif // UTILS_H
