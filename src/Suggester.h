#ifndef SUGGESTER_H
#define SUGGESTER_H

#include <QtGui>

class Suggester : public QObject {

    Q_OBJECT

public:
    Suggester(QObject *parent = 0) : QObject(parent) { }
    virtual void suggest(QString query) = 0;

signals:
    void ready(QStringList);

};

#endif // SUGGESTER_H
