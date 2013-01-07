#ifndef REGIONSVIEW_H
#define REGIONSVIEW_H

#include <QtGui>
#include "view.h"

class YTRegion;

class RegionsView : public QWidget, public View {

    Q_OBJECT

public:
    RegionsView(QWidget *parent = 0);
    void appear();

signals:
    void regionChanged();

private slots:
    void buttonClicked();

private:
    void addRegion(const YTRegion &region);
    QGridLayout *layout;
    QPushButton *doneButton;

};

#endif // REGIONSVIEW_H
