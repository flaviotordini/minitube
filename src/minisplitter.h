#ifndef MINISPLITTER_H
#define MINISPLITTER_H

#include <QtWidgets>

class MiniSplitter : public QSplitter {

public:
    MiniSplitter(Qt::Orientation orientation = Qt::Horizontal, QWidget *parent = 0);

protected:
    QSplitterHandle *createHandle();

};

#endif // MINISPLITTER_H
