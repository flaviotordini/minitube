#include "downloadview.h"
#include "downloadmodel.h"
#include "downloadmanager.h"
#include "downloadlistview.h"
#include "downloaditem.h"
#include "downloadsettings.h"
#include "ListModel.h"
#include "playlist/PrettyItemDelegate.h"
#include "thlibrary/thblackbar.h"

DownloadView::DownloadView(QWidget *parent) : QWidget(parent) {

    QBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);

    bar = new THBlackBar(this);
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
    PrettyItemDelegate *delegate = new PrettyItemDelegate(this, true);
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
