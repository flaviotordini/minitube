#include "SearchView.h"
#include "constants.h"
#include "fontutils.h"
#include "searchparams.h"
#include "youtubesuggest.h"
#include "channelsuggest.h"

namespace The {
    QMap<QString, QAction*>* globalActions();
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

    QBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    // hidden message widget
    message = new QLabel(this);
    message->hide();
    mainLayout->addWidget(message);

#ifdef APP_DEMO
    QLabel *buy = new QLabel(this);
    buy->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    buy->setText(QString("<a style='color:palette(text);text-decoration:none' href='%1'>%2</a>").arg(
            QString(Constants::WEBSITE) + "#download",
            tr("Get the full version").toUpper()
            ));
    buy->setOpenExternalLinks(true);
    buy->setMargin(7);
    buy->setAlignment(Qt::AlignRight);
    buy->setStyleSheet("QLabel {"
                       "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #CCD6E0, stop: 1 #ADBCCC);"
                       "border-bottom-left-radius: 8px;"
                       "border-bottom-right-radius: 8px;"
                       "font-size: 10px;"
                       "margin-right: 50px;"
                       "}");
    mainLayout->addWidget(buy, 0, Qt::AlignRight);
#endif

    mainLayout->addStretch();
    mainLayout->addSpacing(PADDING);

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
            new QLabel("<h1 style='font-weight:normal'>" +
                       tr("Welcome to <a href='%1'>%2</a>,")
                       // .replace("<a ", "<a style='color:palette(text)'")
                       .replace("<a href", "<a style='text-decoration:none; color:palette(text); font-weight:bold' href")
                       .arg(Constants::WEBSITE, Constants::APP_NAME)
                       + "</h1>", this);
    welcomeLabel->setOpenExternalLinks(true);
    layout->addWidget(welcomeLabel);

    layout->addSpacing(PADDING / 2);

    QBoxLayout *tipLayout = new QHBoxLayout();
    tipLayout->setSpacing(10);

    //: "Enter", as in "type". The whole frase says: "Enter a keyword to start watching videos"
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
    queryEdit->setFont(biggerFont);
    queryEdit->setMinimumWidth(queryEdit->fontInfo().pixelSize()*15);
    queryEdit->sizeHint();
    queryEdit->setFocus(Qt::OtherFocusReason);
    connect(queryEdit, SIGNAL(search(const QString&)), this, SLOT(watch(const QString&)));
    connect(queryEdit, SIGNAL(textChanged(const QString &)), this, SLOT(textChanged(const QString &)));

    youtubeSuggest = new YouTubeSuggest(this);
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

    recentKeywordsLayout = new QVBoxLayout();
    recentKeywordsLayout->setSpacing(5);
    recentKeywordsLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    recentKeywordsLabel = new QLabel(tr("Recent keywords").toUpper(), this);
#if defined(APP_MAC) | defined(APP_WIN)
    QPalette palette = recentKeywordsLabel->palette();
    palette.setColor(QPalette::WindowText, QColor(0x65, 0x71, 0x80));
    recentKeywordsLabel->setPalette(palette);
#else
    recentKeywordsLabel->setForegroundRole(QPalette::Dark);
#endif
    recentKeywordsLabel->hide();
    recentKeywordsLabel->setFont(smallerFont);
    recentKeywordsLayout->addWidget(recentKeywordsLabel);

    otherLayout->addLayout(recentKeywordsLayout);

    // recent channels
    recentChannelsLayout = new QVBoxLayout();
    recentChannelsLayout->setSpacing(5);
    recentChannelsLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    recentChannelsLabel = new QLabel(tr("Recent channels").toUpper(), this);
#if defined(APP_MAC) | defined(APP_WIN)
    palette = recentChannelsLabel->palette();
    palette.setColor(QPalette::WindowText, QColor(0x65, 0x71, 0x80));
    recentChannelsLabel->setPalette(palette);
#else
    recentChannelsLabel->setForegroundRole(QPalette::Dark);
#endif
    recentChannelsLabel->hide();
    recentChannelsLabel->setFont(smallerFont);
    recentChannelsLayout->addWidget(recentChannelsLabel);

    otherLayout->addLayout(recentChannelsLayout);

    layout->addLayout(otherLayout);

    mainLayout->addSpacing(PADDING);
    mainLayout->addStretch();

    setLayout(mainLayout);

    updateChecker = 0;
    checkForUpdate();
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
        if (keyword.startsWith("http://")) {
            int separator = keyword.indexOf("|");
            if (separator > 0 && separator + 1 < keyword.length()) {
                link = keyword.left(separator);
                display = keyword.mid(separator+1);
            }
        }
        QLabel *itemLabel = new QLabel("<a href=\"" + link
                                       + "\" style=\"color:palette(text); text-decoration:none\">"
                                       + display + "</a>", this);

        itemLabel->setMaximumWidth(queryEdit->width() + watchButton->width());
        // itemLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        // Make links navigable with the keyboard too
        itemLabel->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard | Qt::LinksAccessibleByMouse);

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
        if (keyword.startsWith("http://")) {
            int separator = keyword.indexOf("|");
            if (separator > 0 && separator + 1 < keyword.length()) {
                link = keyword.left(separator);
                display = keyword.mid(separator+1);
            }
        }
        QLabel *itemLabel = new QLabel("<a href=\"" + link
                                       + "\" style=\"color:palette(text); text-decoration:none\">"
                                       + display + "</a>", this);

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
        query = query.replace(" ", "");
        searchParams->setAuthor(query);
        searchParams->setSortBy(SearchParams::SortByNewest);
    }

    // go!
    emit search(searchParams);
}

void SearchView::watchChannel(QString channel) {

    channel = channel.simplified();

    // check for empty query
    if (channel.length() == 0) {
        queryEdit->setFocus(Qt::OtherFocusReason);
        return;
    }

    // remove spaces from channel name
    channel = channel.replace(" ", "");

    SearchParams *searchParams = new SearchParams();
    searchParams->setAuthor(channel);
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

    SearchParams *searchParams = new SearchParams();
    searchParams->setKeywords(query);

    // go!
    emit search(searchParams);
}

void SearchView::checkForUpdate() {
    static const QString updateCheckKey = "updateCheck";

    // check every 24h
    QSettings settings;
    uint unixTime = QDateTime::currentDateTime().toTime_t();
    int lastCheck = settings.value(updateCheckKey).toInt();
    int secondsSinceLastCheck = unixTime - lastCheck;
    // qDebug() << "secondsSinceLastCheck" << unixTime << lastCheck << secondsSinceLastCheck;
    if (secondsSinceLastCheck < 86400) return;

    // check it out
    if (updateChecker) delete updateChecker;
    updateChecker = new UpdateChecker();
    connect(updateChecker, SIGNAL(newVersion(QString)),
            this, SLOT(gotNewVersion(QString)));
    updateChecker->checkForUpdate();
    settings.setValue(updateCheckKey, unixTime);

}

void SearchView::gotNewVersion(QString version) {
    message->setText(
            tr("A new version of %1 is available. Please <a href='%2'>update to version %3</a>")
            .replace("<a href", "<a style='text-decoration:none; color:palette(text); font-weight:bold' href")
            .arg(
                    Constants::APP_NAME,
                    QString(Constants::WEBSITE).append("#download"),
                    version)
            );
    message->setOpenExternalLinks(true);
    message->setMargin(10);
    message->setAlignment(Qt::AlignCenter);
    // message->setBackgroundRole(QPalette::ToolTipBase);
    // message->setForegroundRole(QPalette::ToolTipText);
    // message->setAutoFillBackground(true);
    message->setStyleSheet("QLabel { border-bottom: 1px solid palette(mid); }");
    message->show();
    if (updateChecker) delete updateChecker;
}

void SearchView::paintEvent(QPaintEvent * /*event*/) {
#if defined(APP_MAC) | defined(APP_WIN)
    QBrush brush;
    if (window()->isActiveWindow()) {
        brush = QBrush(QColor(0xdd, 0xe4, 0xeb));
    } else {
        brush = palette().window();
    }
    QPainter painter(this);
    painter.fillRect(0, 0, width(), height(), brush);
#endif
}

void SearchView::searchTypeChanged(int index) {
    if (index == 0) {
        queryEdit->setSuggester(youtubeSuggest);
    } else {
        queryEdit->setSuggester(channelSuggest);
    }
    queryEdit->setFocus();
}
