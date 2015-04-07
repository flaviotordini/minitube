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
#ifdef APP_MAC
#include "searchlineedit_mac.h"
#else
#include "searchlineedit.h"
#endif
#ifdef APP_EXTRA
#include "extra.h"
#endif
#ifdef APP_ACTIVATION
#include "activation.h"
#endif
#include "mainwindow.h"
#include "painterutils.h"

namespace The {
QHash<QString, QAction*>* globalActions();
}

static const QString recentKeywordsKey = "recentKeywords";
static const QString recentChannelsKey = "recentChannels";
static const int PADDING = 30;

SearchView::SearchView(QWidget *parent) : QWidget(parent) {

    QFont biggerFont = FontUtils::big();
    QFont smallerFont = FontUtils::smallBold();

#if defined(APP_MAC) | defined(APP_WIN)
    // speedup painting since we'll paint the whole background
    // by ourselves anyway in paintEvent()
    setAttribute(Qt::WA_OpaquePaintEvent);
#endif

    setAutoFillBackground(true);

    QBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(PADDING);
    mainLayout->setSpacing(0);

    // hidden message widget
    message = new QLabel(this);
    message->hide();
    mainLayout->addWidget(message);

    mainLayout->addStretch();

    QBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setAlignment(Qt::AlignCenter);
    mainLayout->addLayout(hLayout);

    QLabel *logo = new QLabel(this);
    logo->setPixmap(QPixmap(":/images/app.png"));
    hLayout->addWidget(logo, 0, Qt::AlignTop);
    hLayout->addSpacing(PADDING);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setAlignment(Qt::AlignCenter);
    hLayout->addLayout(layout);

    QLabel *welcomeLabel =
            new QLabel("<h1 style='font-weight:100'>" +
                       tr("Welcome to <a href='%1'>%2</a>,")
                       // .replace("<a ", "<a style='color:palette(text)'")
                       .replace("<a ", "<a style='text-decoration:none; color:palette(text);font-weight:"
                            #if defined(APP_UBUNTU) || defined(APP_WIN)
                                "normal"
                            #else
                                "normal"
                            #endif
                                "' ")
                       .arg(Constants::WEBSITE, Constants::NAME)
                       + "</h1>", this);
    welcomeLabel->setOpenExternalLinks(true);
#ifdef APP_WIN
    QFont f = welcomeLabel->font();
    f.setHintingPreference(QFont::PreferNoHinting);
    f.setFamily("Segoe UI Light");
    welcomeLabel->setFont(f);
#endif
    layout->addWidget(welcomeLabel);

    layout->addSpacing(PADDING / 2);

    QBoxLayout *tipLayout = new QHBoxLayout();
    tipLayout->setSpacing(10);

    //: "Enter", as in "type". The whole phrase says: "Enter a keyword to start watching videos"
    QLabel *tipLabel = new QLabel(tr("Enter"), this);
    tipLabel->setFont(biggerFont);
    tipLayout->addWidget(tipLabel);

    typeCombo = new QComboBox(this);
    typeCombo->addItem(tr("a keyword"));
    typeCombo->addItem(tr("a channel"));
    typeCombo->setFont(biggerFont);
    connect(typeCombo, SIGNAL(currentIndexChanged(int)), SLOT(searchTypeChanged(int)));
    tipLayout->addWidget(typeCombo);

    tipLabel = new QLabel(tr("to start watching videos."), this);
    tipLabel->setFont(biggerFont);
    tipLayout->addWidget(tipLabel);
    layout->addLayout(tipLayout);

    layout->addSpacing(PADDING / 2);

    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLayout->setAlignment(Qt::AlignVCenter);

    queryEdit = new SearchLineEdit(this);
#ifndef APP_MAC
    queryEdit->setFont(biggerFont);
#endif
    connect(queryEdit, SIGNAL(search(const QString&)), SLOT(watch(const QString&)));
    connect(queryEdit, SIGNAL(textEdited(const QString &)), SLOT(textChanged(const QString &)));
    connect(queryEdit, SIGNAL(suggestionAccepted(Suggestion*)), SLOT(suggestionAccepted(Suggestion*)));

    youtubeSuggest = new YTSuggester(this);
    channelSuggest = new ChannelSuggest(this);
    searchTypeChanged(0);

    searchLayout->addWidget(queryEdit);
    searchLayout->addSpacing(10);

    watchButton = new QPushButton(tr("Watch"), this);
    watchButton->setDefault(true);
    watchButton->setEnabled(false);
    watchButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(watchButton, SIGNAL(clicked()), this, SLOT(watch()));
    searchLayout->addWidget(watchButton);

    layout->addItem(searchLayout);

    layout->addSpacing(PADDING / 2);

    QHBoxLayout *otherLayout = new QHBoxLayout();
    otherLayout->setMargin(0);
    otherLayout->setSpacing(10);

    recentKeywordsLayout = new QVBoxLayout();
    recentKeywordsLayout->setSpacing(5);
    recentKeywordsLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    recentKeywordsLabel = new QLabel(tr("Recent keywords").toUpper(), this);
    recentKeywordsLabel->setProperty("recentHeader", true);
    recentKeywordsLabel->setForegroundRole(QPalette::Dark);
    recentKeywordsLabel->hide();
    recentKeywordsLabel->setFont(smallerFont);
    recentKeywordsLayout->addWidget(recentKeywordsLabel);

    otherLayout->addLayout(recentKeywordsLayout);

    // recent channels
    recentChannelsLayout = new QVBoxLayout();
    recentChannelsLayout->setSpacing(5);
    recentChannelsLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    recentChannelsLabel = new QLabel(tr("Recent channels").toUpper(), this);
    recentChannelsLabel->setProperty("recentHeader", true);
    recentChannelsLabel->setForegroundRole(QPalette::Dark);
    recentChannelsLabel->hide();
    recentChannelsLabel->setFont(smallerFont);
    recentChannelsLayout->addWidget(recentChannelsLabel);

    otherLayout->addLayout(recentChannelsLayout);

    layout->addLayout(otherLayout);

    mainLayout->addStretch();

#ifdef APP_ACTIVATION
    if (!Activation::instance().isActivated())
        mainLayout->addWidget(Extra::buyButton(tr("Get the full version")), 0, Qt::AlignRight);
#endif
}

void SearchView::appear() {
    setUpdatesEnabled(false);
    updateRecentKeywords();
    updateRecentChannels();
    queryEdit->selectAll();
    queryEdit->enableSuggest();
    if (!queryEdit->hasFocus())
        QTimer::singleShot(10, queryEdit, SLOT(setFocus()));
    setUpdatesEnabled(true);
}

void SearchView::updateRecentKeywords() {

    // cleanup
    QLayoutItem *item;
    while ((item = recentKeywordsLayout->takeAt(1)) != 0) {
        item->widget()->close();
        delete item;
    }

    // load
    QSettings settings;
    QStringList keywords = settings.value(recentKeywordsKey).toStringList();
    recentKeywordsLabel->setVisible(!keywords.isEmpty());
    The::globalActions()->value("clearRecentKeywords")->setEnabled(!keywords.isEmpty());

    foreach (QString keyword, keywords) {
        QString link = keyword;
        QString display = keyword;
        if (keyword.startsWith("http://") || keyword.startsWith("https://")) {
            int separator = keyword.indexOf("|");
            if (separator > 0 && separator + 1 < keyword.length()) {
                link = keyword.left(separator);
                display = keyword.mid(separator+1);
            }
        }
        bool needStatusTip = false;
        if (display.length() > 24) {
            display.truncate(24);
            display.append("...");
            needStatusTip = true;
        }
        QLabel *itemLabel = new QLabel("<a href=\"" + link
                                       + "\" style=\"color:palette(text); text-decoration:none\">"
                                       + display + "</a>", this);
        itemLabel->setAttribute(Qt::WA_DeleteOnClose);
        itemLabel->setProperty("recentItem", true);
        itemLabel->setMaximumWidth(queryEdit->width() + watchButton->width());
        // itemLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        // Make links navigable with the keyboard too
        itemLabel->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard | Qt::LinksAccessibleByMouse);
        if (needStatusTip)
            itemLabel->setStatusTip(link);
        connect(itemLabel, SIGNAL(linkActivated(QString)), this, SLOT(watchKeywords(QString)));
        recentKeywordsLayout->addWidget(itemLabel);
    }

}

void SearchView::updateRecentChannels() {

    // cleanup
    QLayoutItem *item;
    while ((item = recentChannelsLayout->takeAt(1)) != 0) {
        item->widget()->close();
        delete item;
    }

    // load
    QSettings settings;
    QStringList keywords = settings.value(recentChannelsKey).toStringList();
    recentChannelsLabel->setVisible(!keywords.isEmpty());
    // TODO The::globalActions()->value("clearRecentKeywords")->setEnabled(!keywords.isEmpty());

    foreach (QString keyword, keywords) {
        QString link = keyword;
        QString display = keyword;
        int separator = keyword.indexOf('|');
        if (separator > 0 && separator + 1 < keyword.length()) {
            link = keyword.left(separator);
            display = keyword.mid(separator+1);
        }
        QLabel *itemLabel = new QLabel("<a href=\"" + link
                                       + "\" style=\"color:palette(text); text-decoration:none\">"
                                       + display + "</a>", this);
        itemLabel->setAttribute(Qt::WA_DeleteOnClose);
        itemLabel->setProperty("recentItem", true);
        itemLabel->setMaximumWidth(queryEdit->width() + watchButton->width());
        // itemLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        // Make links navigable with the keyboard too
        itemLabel->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard | Qt::LinksAccessibleByMouse);

        connect(itemLabel, SIGNAL(linkActivated(QString)), this, SLOT(watchChannel(QString)));
        recentChannelsLayout->addWidget(itemLabel);
    }

}

void SearchView::watch() {
    QString query = queryEdit->text();
    watch(query);
}

void SearchView::textChanged(const QString &text) {
    watchButton->setEnabled(!text.simplified().isEmpty());
}

void SearchView::watch(QString query) {

    query = query.simplified();

    // check for empty query
    if (query.length() == 0) {
        queryEdit->setFocus(Qt::OtherFocusReason);
        return;
    }

    SearchParams *searchParams = new SearchParams();
    if (typeCombo->currentIndex() == 0)
        searchParams->setKeywords(query);
    else {
        // remove spaces from channel name
        query = query.simplified();
        query = query.remove(' ');
        searchParams->setChannelId(query);
        searchParams->setSortBy(SearchParams::SortByNewest);
    }

    // go!
    emit search(searchParams);
}

void SearchView::watchChannel(const QString &channelId) {
    if (channelId.length() == 0) {
        queryEdit->setFocus(Qt::OtherFocusReason);
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

void SearchView::watchKeywords(QString query) {

    query = query.simplified();

    // check for empty query
    if (query.length() == 0) {
        queryEdit->setFocus(Qt::OtherFocusReason);
        return;
    }

    if (typeCombo->currentIndex() == 0) {
        queryEdit->setText(query);
        watchButton->setEnabled(true);
    }

    SearchParams *searchParams = new SearchParams();
    searchParams->setKeywords(query);

    // go!
    emit search(searchParams);
}

void SearchView::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);
#if defined(APP_MAC) | defined(APP_WIN)
    QBrush brush;
    if (window()->isActiveWindow()) {
        brush = QBrush(QColor(0xdd, 0xe4, 0xeb));
    } else {
        brush = palette().window();
    }
    QPainter painter(this);
    painter.fillRect(0, 0, width(), height(), brush);
    painter.end();
#endif
#ifdef APP_UBUNTU
    QStyleOption o;
    o.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &o, &p, this);
#endif
    PainterUtils::topShadow(this);
}

void SearchView::searchTypeChanged(int index) {
    if (index == 0) {
        queryEdit->setSuggester(youtubeSuggest);
    } else {
        queryEdit->setSuggester(channelSuggest);
    }
    queryEdit->selectAll();
    queryEdit->setFocus();
}

void SearchView::suggestionAccepted(Suggestion *suggestion) {
    if (suggestion->type == QLatin1String("channel")) {
        watchChannel(suggestion->userData);
    } else watch(suggestion->value);
}
