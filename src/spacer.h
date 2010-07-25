#ifndef SPACER_H
#define SPACER_H

#include <QtGui>

class Spacer : public QWidget {

public:
    Spacer(QWidget *parent = 0);

protected:
    QSize sizeHint() const;
};

#endif // SPACER_H
