#include "sidebarheader.h"
#include "utils.h"
#include "mediaview.h"
#include "videosource.h"
#include "fontutils.h"

SidebarHeader::SidebarHeader(QWidget *parent) : QToolBar(parent) { }

void SidebarHeader::setup() {
    static bool isSetup = false;
    if (isSetup) return;
    isSetup = true;

    backAction = new QAction(
                Utils::icon("go-previous"),
                tr("&Back"), this);
    connect(backAction, SIGNAL(triggered()), MediaView::instance(), SLOT(goBack()));
    addAction(backAction);

    forwardAction = new QAction(
                Utils::icon("go-next"),
                tr("&Back"), this);
    connect(forwardAction, SIGNAL(triggered()), MediaView::instance(), SLOT(goForward()));
    addAction(forwardAction);

    /*
    QWidget *spacerWidget = new QWidget(this);
    spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    spacerWidget->setVisible(true);
    addWidget(spacerWidget);
    */
}

QSize SidebarHeader::minimumSizeHint (void) const {
    return(QSize(1, QFontMetrics(font()).height() * 1.9));
}

void SidebarHeader::updateInfo() {
    setup();

    QList<VideoSource*> history = MediaView::instance()->getHistory();
    int currentIndex = MediaView::instance()->getHistoryIndex();

    bool canGoForward = MediaView::instance()->canGoForward();
    forwardAction->setVisible(canGoForward);
    if (canGoForward) {
        VideoSource *nextVideoSource = history.at(currentIndex + 1);
        forwardAction->setStatusTip(
                    tr("Forward to %1")
                    .arg(nextVideoSource->getName()));
    }

    bool canGoBack = MediaView::instance()->canGoBack();
    bool backVisible = canGoForward || canGoBack;
    backAction->setVisible(backVisible);
    backAction->setEnabled(canGoBack);
    if (canGoBack) {
        VideoSource *previousVideoSource = history.at(currentIndex - 1);
        backAction->setStatusTip(
                    tr("Back to %1")
                    .arg(previousVideoSource->getName()));
    }

    VideoSource *currentVideoSource = history.at(currentIndex);
    connect(currentVideoSource, SIGNAL(nameChanged(QString)),
            SLOT(updateTitle(QString)), Qt::UniqueConnection);
    setTitle(currentVideoSource->getName());
}

void SidebarHeader::updateTitle(QString title) {
    sender()->disconnect(this);
    setTitle(title);
}

void SidebarHeader::setTitle(QString title) {
    this->title = title;
    update();
}

void SidebarHeader::paintEvent(QPaintEvent *event) {
    QToolBar::paintEvent(event);
    if (title.isEmpty()) return;
    QPainter p(this);
    p.setFont(FontUtils::smallBold());
    p.setPen(Qt::white);

    const QRect r = rect();

    QString t = title;
    QRect textBox = p.boundingRect(r, Qt::AlignCenter, t);
    int i = 1;
    static const int margin = 100;
    while (textBox.width() > r.width() - margin) {
        t = t.left(t.length() - i) + "...";
        textBox = p.boundingRect(r, Qt::AlignCenter, t);
        i++;
    }

    p.drawText(r, Qt::AlignCenter, t);
}
