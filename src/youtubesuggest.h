#ifndef YOUTUBESUGGEST_H
#define YOUTUBESUGGEST_H

#include <QtCore>

#include "suggester.h"

class YouTubeSuggest : public Suggester {

    Q_OBJECT

public:
    YouTubeSuggest(QObject *parent = 0);
    void suggest(QString query);

signals:
    void ready(QStringList);

private slots:
    void handleNetworkData(QByteArray response);

};

#endif // YOUTUBESUGGEST_H
