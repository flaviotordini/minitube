#include "playlistwidget.h"
#include "segmentedcontrol.h"

PlaylistWidget::PlaylistWidget (QWidget *parent, SegmentedControl *tabBar, QListView *listView)
    : QWidget(parent) {
    QBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(tabBar);
    layout->addWidget(listView);
}
