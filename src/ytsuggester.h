#ifndef YTSUGGESTER_H
#define YTSUGGESTER_H

#include <QtCore>
#include "suggester.h"

class YTSuggester : public Suggester {

    Q_OBJECT

public:
    YTSuggester(QObject *parent = 0);
    void suggest(QString query);

signals:
    void ready(QStringList);

private slots:
    void handleNetworkData(QByteArray response);

};

#endif // YTSUGGESTER_H
