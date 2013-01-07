#ifndef USERVIEW_H
#define USERVIEW_H

#include <QtGui>
#include "view.h"

class VideoSource;

class UserView : public QWidget, public View {

    Q_OBJECT

public:
    UserView(QWidget *parent = 0);

signals:
    void activated(VideoSource *standardFeed);
    
private:
    QGridLayout *layout;
    
};

#endif // USERVIEW_H
