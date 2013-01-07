#ifndef PLAYLISTSUGGEST_H
#define PLAYLISTSUGGEST_H

#include <QtGui>
#include "suggester.h"

class PlaylistSuggest : public Suggester {

    Q_OBJECT

public:
    PlaylistSuggest(QObject *parent = 0);
    void suggest(QString query);

signals:
    void ready(QStringList);

private slots:
    void handleNetworkData(QByteArray response);

};

#endif // PLAYLISTSUGGEST_H
