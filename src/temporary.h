#ifndef TEMPORARY_H
#define TEMPORARY_H

#include <QtCore>
#include <QDesktopServices>

class Temporary {

public:
    static QString filename();
    static void deleteAll();

private:
    Temporary();

};

#endif // TEMPORARY_H
