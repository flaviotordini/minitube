#ifndef DATAUTILS_H
#define DATAUTILS_H

#include <QtCore>

class DataUtils {

public:
    static QString stringToFilename(const QString &s);
    static QString regioneCode(const QLocale &locale);
    static QString systemRegioneCode();
    static uint parseIsoPeriod(const QString &isoPeriod);

private:
    DataUtils() { }

};

#endif // DATAUTILS_H
