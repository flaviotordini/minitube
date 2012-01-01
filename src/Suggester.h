#ifndef SUGGESTER_H
#define SUGGESTER_H

#include <QtGui>

class Suggester : public QObject {

    Q_OBJECT

public:
    virtual void suggest(QString query) = 0;

signals:
    void ready(QStringList);

};

#endif // SUGGESTER_H
