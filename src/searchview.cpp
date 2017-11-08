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

#include "searchview.h"
#include "constants.h"
#include "fontutils.h"
#include "searchparams.h"
#include "ytsuggester.h"
#include "channelsuggest.h"
#ifdef APP_MAC_SEARCHFIELD
#include "searchlineedit_mac.h"
#else
#include "searchlineedit.h"
#endif
#ifdef APP_EXTRA
#include "extra.h"
#endif
#ifdef APP_ACTIVATION
#include "activation.h"
#include "activationview.h"
#endif
#include "mainwindow.h"
#include "painterutils.h"
#include "iconutils.h"
#include "clickablelabel.h"

namespace {
static const QString recentKeywordsKey = "recentKeywords";
static const QString recentChannelsKey = "recentChannels";
}

SearchView::SearchView(QWidget *parent) : View(parent) {
    const int padding = 30;

    // speedup painting since we'll paint the whole background
    // by ourselves anyway in paintEvent()
    setAttribute(Qt::WA_OpaquePaintEvent);

    QBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setMargin(padding);
    vLayout->setSpacing(0);

    // hidden message widget
    message = new QLabel(this);
    message->hide();
    vLayout->addWidget(message);

    vLayout->addStretch();

    QBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setAlignment(Qt::AlignCenter);

    vLayout->addLayout(hLayout);

    hLayout->addStretch();

    logo = new ClickableLabel(this);
    logo->setPixmap(IconUtils::pixmap(":/images/app.png"));
    connect(logo, &ClickableLabel::clicked, MainWindow::instance(), &MainWindow::visitSite);
    hLayout->addWidget(logo, 0, Qt::AlignTop);
    hLayout->addSpacing(padding);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setAlignment(Qt::AlignCenter);
    hLayout->addLayout(layout);

    QColor titleColor = palette().color(QPalette::WindowText);
    titleColor.setAlphaF(.75);
    int r,g,b,a;
    titleColor.getRgb(&r,&g,&b,&a);
    QString cssColor = QString::asprintf("rgba(%d,%d,%d,%d)", r, g, b, a);

    QLabel *welcomeLabel =
            new QLabel(QString("<h1 style='font-weight:300;color:%1'>").arg(cssColor) +
                       tr("Welcome to <a href='%1'>%2</a>,")
                       .replace("<a ", "<a style='text-decoration:none; color:palette(text)' ")
                       .arg(Constants::WEBSITE, Constants::NAME)
                       + "</h1>");
    welcomeLabel->setOpenExternalLinks(true);
    welcomeLabel->setProperty("heading", true);
    welcomeLabel->setFont(FontUtils::light(welcomeLabel->font().pointSize() * 1.25));
    layout->addWidget(welcomeLabel);

    layout->addSpacing(padding);

    QBoxLayout *tipLayout = new QHBoxLayout();
    tipLayout->setAlignment(Qt::AlignLeft);
    tipLayout->setSpacing(10);

#ifndef APP_MAC
    const QFont &biggerFont = FontUtils::big();
#endif

    //: "Enter", as in "type". The whole phrase says: "Enter a keyword to start watching videos"
    QLabel *tipLabel = new QLabel(tr("Enter"), this);
#ifndef APP_MAC
    tipLabel->setFont(biggerFont);
#endif
    tipLayout->addWidget(tipLabel);

    typeCombo = new QComboBox(this);
    typeCombo->addItem(tr("a keyword"));
    typeCombo->addItem(tr("a channel"));
#ifndef APP_MAC
    typeCombo->setFont(biggerFont);
#endif
    connect(typeCombo, SIGNAL(currentIndexChanged(int)), SLOT(searchTypeChanged(int)));
    tipLayout->addWidget(typeCombo);

    tipLabel = new QLabel(tr("to start watching videos."), this);
#ifndef APP_MAC
    tipLabel->setFont(biggerFont);
#endif
    tipLayout->addWidget(tipLabel);
    layout->addLayout(tipLayout);

    layout->addSpacing(padding / 2);

    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLayout->setAlignment(Qt::AlignVCenter);

#ifdef APP_MAC_SEARCHFIELD
    SearchLineEditMac *slem = new SearchLineEditMac(this);
    queryEdit = slem;
    setFocusProxy(slem);
#else
    SearchLineEdit *sle = new SearchLineEdit(this);
    sle->setFont(biggerFont);
    queryEdit = sle;
#endif

    connect(queryEdit->toWidget(), SIGNAL(search(const QString&)), SLOT(watch(const QString&)));
    connect(queryEdit->toWidget(), SIGNAL(textChanged(const QString &)), SLOT(textChanged(const QString &)));
    connect(queryEdit->toWidget(), SIGNAL(textEdited(const QString &)), SLOT(textChanged(const QString &)));
    connect(queryEdit->toWidget(), SIGNAL(suggestionAccepted(Suggestion*)), SLOT(suggestionAccepted(Suggestion*)));

    youtubeSuggest = new YTSuggester(this);
    channelSuggest = new ChannelSuggest(this);
    connect(channelSuggest, SIGNAL(ready(QVector<Suggestion*>)), SLOT(onChannelSuggestions(QVector<Suggestion*>)));
    searchTypeChanged(0);

    searchLayout->addWidget(queryEdit->toWidget(), 0, Qt::AlignBaseline);
    searchLayout->addSpacing(padding);

    watchButton = new QPushButton(tr("Watch"), this);
#ifndef APP_MAC
    watchButton->setFont(biggerFont);
#endif
    watchButton->setDefault(true);
    watchButton->setEnabled(false);
    watchButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(watchButton, SIGNAL(clicked()), this, SLOT(watch()));
    searchLayout->addWidget(watchButton, 0, Qt::AlignBaseline);

    layout->addItem(searchLayout);

    layout->addSpacing(padding);

    QHBoxLayout *recentLayout = new QHBoxLayout();
    recentLayout->setMargin(5);
    recentLayout->setSpacing(10);

    recentKeywordsLayout = new QVBoxLayout();
    recentKeywordsLayout->setMargin(0);
    recentKeywordsLayout->setSpacing(0);
    recentKeywordsLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    recentKeywordsLabel = new QLabel(tr("Recent keywords"), this);
    recentKeywordsLabel->setProperty("recentHeader", true);
    recentKeywordsLabel->hide();
    recentKeywordsLayout->addWidget(recentKeywordsLabel);

    recentLayout->addLayout(recentKeywordsLayout);

    // recent channels
    recentChannelsLayout = new QVBoxLayout();
    recentChannelsLayout->setMargin(0);
    recentChannelsLayout->setSpacing(0);
    recentChannelsLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    recentChannelsLabel = new QLabel(tr("Recent channels"), this);
    recentChannelsLabel->setProperty("recentHeader", true);
    recentChannelsLabel->hide();
    recentChannelsLayout->addWidget(recentChannelsLabel);

    recentLayout->addLayout(recentChannelsLayout);

    layout->addLayout(recentLayout);

    hLayout->addStretch();

    vLayout->addStretch();

#ifdef APP_ACTIVATION
    if (!Activation::instance().isActivated())
        vLayout->addWidget(ActivationView::buyButton(tr("Get the full version")), 0, Qt::AlignRight);
#endif
}

void SearchView::appear() {
    MainWindow *w = MainWindow::instance();
    w->showActionInStatusBar(w->getActionMap().value("manualplay"), true);
    w->showActionInStatusBar(w->getActionMap().value("safeSearch"), true);
    w->showActionInStatusBar(w->getActionMap().value("definition"), true);

    updateRecentKeywords();
    updateRecentChannels();
    queryEdit->selectAll();
    queryEdit->enableSuggest();

    if (!queryEdit->toWidget()->hasFocus()) queryEdit->toWidget()->setFocus();

    connect(window()->windowHandle(), SIGNAL(screenChanged(QScreen*)), SLOT(screenChanged()), Qt::UniqueConnection);

    qApp->processEvents();
    update();

#ifdef APP_MAC
    // Workaround cursor bug on macOS
    window()->unsetCursor();
#endif
}

void SearchView::disappear() {
    MainWindow *w = MainWindow::instance();
    w->showActionInStatusBar(w->getActionMap().value("safeSearch"), false);
    w->showActionInStatusBar(w->getActionMap().value("definition"), false);
    w->showActionInStatusBar(w->getActionMap().value("manualplay"), false);
}

void SearchView::updateRecentKeywords() {
    // load
    QSettings settings;
    QStringList keywords = settings.value(recentKeywordsKey).toStringList();
    if (keywords == recentKeywords) return;
    recentKeywords = keywords;

    // cleanup
    QLayoutItem *item;
    while ((item = recentKeywordsLayout->takeAt(1)) != 0) {
        item->widget()->close();
        delete item;
    }

    recentKeywordsLabel->setVisible(!keywords.isEmpty());
    MainWindow::instance()->getActionMap().value("clearRecentKeywords")->setEnabled(!keywords.isEmpty());

    foreach (const QString &keyword, keywords) {
        QString link = keyword;
        QString display = keyword;
        if (keyword.startsWith(QLatin1String("http://")) || keyword.startsWith(QLatin1String("https://"))) {
            int separator = keyword.indexOf('|');
            if (separator > 0 && separator + 1 < keyword.length()) {
                link = keyword.left(separator);
                display = keyword.mid(separator+1);
            }
        }
        bool needStatusTip = false;
        const int maxDisplayLength = 25;
        if (display.length() > maxDisplayLength) {
            display.truncate(maxDisplayLength);
            display.append(QStringLiteral("\u2026"));
            needStatusTip = true;
        }
        QPushButton *itemButton = new QPushButton(display);
        itemButton->setAttribute(Qt::WA_DeleteOnClose);
        itemButton->setProperty("recentItem", true);
        itemButton->setCursor(Qt::PointingHandCursor);
        itemButton->setFocusPolicy(Qt::TabFocus);
        itemButton->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        if (needStatusTip)
            itemButton->setStatusTip(link);
        connect(itemButton, &QPushButton::clicked, [this,link]() {
            watchKeywords(link);
        });

        recentKeywordsLayout->addWidget(itemButton);
    }

}

void SearchView::updateRecentChannels() {
    // load
    QSettings settings;
    QStringList keywords = settings.value(recentChannelsKey).toStringList();
    if (keywords == recentChannels) return;
    recentChannels = keywords;

    // cleanup
    QLayoutItem *item;
    while ((item = recentChannelsLayout->takeAt(1)) != 0) {
        item->widget()->close();
        delete item;
    }

    recentChannelsLabel->setVisible(!keywords.isEmpty());
    // TODO MainWindow::instance()->getActionMap().value("clearRecentKeywords")->setEnabled(!keywords.isEmpty());

    foreach (const QString &keyword, keywords) {
        QString link = keyword;
        QString display = keyword;
        int separator = keyword.indexOf('|');
        if (separator > 0 && separator + 1 < keyword.length()) {
            link = keyword.left(separator);
            display = keyword.mid(separator+1);
        }
        QPushButton *itemButton = new QPushButton(display);
        itemButton->setAttribute(Qt::WA_DeleteOnClose);
        itemButton->setProperty("recentItem", true);
        itemButton->setCursor(Qt::PointingHandCursor);
        itemButton->setFocusPolicy(Qt::TabFocus);
        itemButton->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        connect(itemButton, &QPushButton::clicked, [this,link]() {
            watchChannel(link);
        });
        recentChannelsLayout->addWidget(itemButton);
    }
}

void SearchView::watch() {
    QString query = queryEdit->text();
    watch(query);
}

void SearchView::textChanged(const QString &text) {
    watchButton->setEnabled(!text.simplified().isEmpty());
    lastChannelSuggestions.clear();
}

void SearchView::watch(const QString &query) {
    QString q = query.simplified();

    // check for empty query
    if (q.length() == 0) {
        queryEdit->toWidget()->setFocus(Qt::OtherFocusReason);
        return;
    }

    SearchParams *searchParams = new SearchParams();
    if (typeCombo->currentIndex() == 0)
        searchParams->setKeywords(q);
    else {
        if (lastChannelSuggestions.isEmpty())
            MainWindow::instance()->showMessage(tr("Pick a channel from the suggestions"));
        else
            suggestionAccepted(lastChannelSuggestions.first());
        return;
    }

    // go!
    emit search(searchParams);
}

void SearchView::watchChannel(const QString &channelId) {
    if (channelId.length() == 0) {
        queryEdit->toWidget()->setFocus(Qt::OtherFocusReason);
        return;
    }

    QString id = channelId;

    // Fix old settings
    if (!id.startsWith("UC")) id = "UC" + id;

    SearchParams *searchParams = new SearchParams();
    searchParams->setChannelId(id);
    searchParams->setSortBy(SearchParams::SortByNewest);

    // go!
    emit search(searchParams);
}

void SearchView::watchKeywords(const QString &query) {
    QString q = query.simplified();

    // check for empty query
    if (query.length() == 0) {
        queryEdit->toWidget()->setFocus(Qt::OtherFocusReason);
        return;
    }

    if (typeCombo->currentIndex() == 0) {
        queryEdit->setText(q);
        watchButton->setEnabled(true);
    }

    SearchParams *searchParams = new SearchParams();
    searchParams->setKeywords(q);

    // go!
    emit search(searchParams);
}

void SearchView::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);
    QBrush brush;
    if (window()->isActiveWindow()) {
        brush = palette().base();
    } else {
        brush = palette().window();
    }
    QPainter painter(this);
    painter.fillRect(0, 0, width(), height(), brush);
}

void SearchView::searchTypeChanged(int index) {
    if (index == 0) {
        queryEdit->setSuggester(youtubeSuggest);
    } else {
        queryEdit->setSuggester(channelSuggest);
    }
    queryEdit->selectAll();
    queryEdit->toWidget()->setFocus();
}

void SearchView::suggestionAccepted(Suggestion *suggestion) {
    if (suggestion->type == QLatin1String("channel")) {
        watchChannel(suggestion->userData);
    } else watch(suggestion->value);
}

void SearchView::screenChanged() {
    logo->setPixmap(IconUtils::pixmap(":/images/app.png"));
}

void SearchView::onChannelSuggestions(const QVector<Suggestion *> &suggestions) {
    lastChannelSuggestions = suggestions;
}
