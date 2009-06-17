#include "playlistwidget.h"

PlaylistWidget::PlaylistWidget (QWidget *parent, THBlackBar *tabBar, QListView *listView)
    : QWidget(parent) {
    QBoxLayout *layout = new QVBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(tabBar);
    layout->addWidget(listView);
    setLayout(layout);
}
