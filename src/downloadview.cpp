/* $BEGIN_LICENSE

This file is part of Minitube.
Copyright 2009, Flavio Tordini <flavio.tordini@gmail.com>

Minitube is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Minitube is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Minitube.  If not, see <http://www.gnu.org/licenses/>.

$END_LICENSE */

#include "downloadview.h"
#include "downloadmodel.h"
#include "downloadmanager.h"
#include "downloadlistview.h"
#include "downloaditem.h"
#include "downloadsettings.h"
#include "playlistmodel.h"
#include "playlistitemdelegate.h"
#include "segmentedcontrol.h"

DownloadView::DownloadView(QWidget *parent) : View(parent) {

    QBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);

    bar = new SegmentedControl(this);
    QAction *action = new QAction(tr("Downloads"), this);
    bar->addAction(action);
    layout->addWidget(bar);

    listView = new DownloadListView(this);
#ifdef APP_MAC
    listView->setAlternatingRowColors(true);
#endif
    /*
    QPalette p = listView->palette();
    p.setColor(QPalette::Base, palette().color(QPalette::Window));
    listView->setPalette(p);
    */
    PlaylistItemDelegate *delegate = new PlaylistItemDelegate(this, true);
    listView->setItemDelegate(delegate);
    listView->setSelectionMode(QAbstractItemView::NoSelection);

    // cosmetics
    listView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    listView->setFrameShape(QFrame::NoFrame);
    listView->setAttribute(Qt::WA_MacShowFocusRect, false);
    listView->setMinimumSize(320,240);
    listView->setUniformItemSizes(true);

    listModel = DownloadManager::instance()->getModel();
    listView->setModel(listModel);
    connect(listView, SIGNAL(downloadButtonPushed(QModelIndex)), SLOT(buttonPushed(QModelIndex)));
    connect(listView, SIGNAL(entered(const QModelIndex &)), SLOT(itemEntered(const QModelIndex &)));

    layout->addWidget(listView);

    updateTimer = new QTimer(this);
    updateTimer->setInterval(1000);
    connect(updateTimer, SIGNAL(timeout()), listModel, SLOT(sendReset()));

    downloadSettings = new DownloadSettings(this);
    layout->addWidget(downloadSettings);
}

void DownloadView::appear() {
    listView->setEnabled(true);
    listModel->sendReset();
    listView->setMouseTracking(true);
    updateTimer->start();
}

void DownloadView::disappear() {
    listView->setEnabled(false);
    listView->setMouseTracking(false);
}

void DownloadView::itemEntered(const QModelIndex &index) {
    listModel->setHoveredRow(index.row());
}

void DownloadView::buttonPushed(QModelIndex index) {
    const DownloadItemPointer downloadItemPointer = index.data(DownloadItemRole).value<DownloadItemPointer>();
    DownloadItem *downloadItem = downloadItemPointer.data();

    switch (downloadItem->status()) {
    case Downloading:
    case Starting:
        downloadItem->stop();
        break;
    case Idle:
    case Failed:
        downloadItem->tryAgain();
        break;
    case Finished:
        downloadItem->openFolder();
    }

}
