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

#include "sidebarheader.h"
#include "iconutils.h"
#include "mediaview.h"
#include "videosource.h"
#include "fontutils.h"

SidebarHeader::SidebarHeader(QWidget *parent) : QToolBar(parent) { }

void SidebarHeader::setup() {
    static bool isSetup = false;
    if (isSetup) return;
    isSetup = true;

    setIconSize(QSize(16, 16));

    backAction = new QAction(
                IconUtils::icon("go-previous"),
                tr("&Back"), this);
    backAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Left));
    connect(backAction, SIGNAL(triggered()), MediaView::instance(), SLOT(goBack()));
    addAction(backAction);

    forwardAction = new QAction(
                IconUtils::icon("go-next"),
                tr("&Back"), this);
    forwardAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Right));
    connect(forwardAction, SIGNAL(triggered()), MediaView::instance(), SLOT(goForward()));
    addAction(forwardAction);

    foreach (QAction* action, actions()) {
        window()->addAction(action);
        IconUtils::setupAction(action);
    }

    QWidget *spacerWidget = new QWidget(this);
    spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    addWidget(spacerWidget);
}

QSize SidebarHeader::minimumSizeHint() const {
    return QSize(160, QFontMetrics(font()).height() * 1.9);
}

void SidebarHeader::updateInfo() {
    setup();

    const QList<VideoSource*> &history = MediaView::instance()->getHistory();
    int currentIndex = MediaView::instance()->getHistoryIndex();

    bool canGoForward = MediaView::instance()->canGoForward();
    forwardAction->setVisible(canGoForward);
    forwardAction->setEnabled(canGoForward);
    if (canGoForward) {
        VideoSource *nextVideoSource = history.at(currentIndex + 1);
        forwardAction->setStatusTip(
                    tr("Forward to %1")
                    .arg(nextVideoSource->getName())
                    + " (" + forwardAction->shortcut().toString(QKeySequence::NativeText) + ")"
                    );
    }

    bool canGoBack = MediaView::instance()->canGoBack();
    bool backVisible = canGoForward || canGoBack;
    backAction->setVisible(backVisible);
    backAction->setEnabled(canGoBack);
    if (canGoBack) {
        VideoSource *previousVideoSource = history.at(currentIndex - 1);
        backAction->setStatusTip(
                    tr("Back to %1")
                    .arg(previousVideoSource->getName())
                    + " (" + backAction->shortcut().toString(QKeySequence::NativeText) + ")"
                    );
    }

    VideoSource *currentVideoSource = history.at(currentIndex);
    connect(currentVideoSource, SIGNAL(nameChanged(QString)),
            SLOT(updateTitle(QString)), Qt::UniqueConnection);
    setTitle(currentVideoSource->getName());
}

void SidebarHeader::updateTitle(const QString &title) {
    sender()->disconnect(this);
    setTitle(title);
}

void SidebarHeader::setTitle(const QString &title) {
    this->title = title;
    update();

    QList<VideoSource*> history = MediaView::instance()->getHistory();
    int currentIndex = MediaView::instance()->getHistoryIndex();
    VideoSource *currentVideoSource = history.at(currentIndex);
    foreach (QAction* action, videoSourceActions)
        removeAction(action);
    videoSourceActions = currentVideoSource->getActions();
    addActions(videoSourceActions);
}

void SidebarHeader::paintEvent(QPaintEvent *event) {
    QToolBar::paintEvent(event);
    if (title.isEmpty()) return;
    QPainter p(this);
    p.setPen(Qt::white);

    const QRect r = rect();

    QString t = title;
    QRect textBox = p.boundingRect(r, Qt::AlignCenter, t);
    int i = 1;
    const int margin = forwardAction->isVisible() ? 45 : 20;
    while (textBox.width() > r.width() - margin*2 && t.length() > 3) {
        t = t.left(t.length() - i).trimmed() + QStringLiteral("\u2026");
        textBox = p.boundingRect(r, Qt::AlignCenter, t);
        i++;
    }
    p.drawText(r, Qt::AlignCenter, t);


}
