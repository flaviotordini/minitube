#ifndef CHANNELSUGGEST_H
#define CHANNELSUGGEST_H

#include <QtCore>

#include "suggester.h"

class ChannelSuggest : public Suggester {

    Q_OBJECT

public:
    ChannelSuggest(QObject *parent = 0);
    void suggest(QString query);

signals:
    void ready(QStringList);

private slots:
    void handleNetworkData(QByteArray response);

};

#endif // CHANNELSUGGEST_H
