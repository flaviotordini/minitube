#include "SearchView.h"
#include "constants.h"
#include "fontutils.h"

namespace The {
    QMap<QString, QAction*>* globalActions();
}

static const QString recentKeywordsKey = "recentKeywords";
static const int PADDING = 30;

SearchView::SearchView(QWidget *parent) : QWidget(parent) {

    QFont biggerFont = FontUtils::big();
    QFont smallerFont = FontUtils::smallBold();

#ifdef APP_MAC
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

    QLabel *tipLabel = new QLabel(tr("Enter a keyword to start watching videos."), this);
    tipLabel->setFont(biggerFont);
    layout->addWidget(tipLabel);

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
    recentKeywordsLayout->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    recentKeywordsLabel = new QLabel(tr("Recent keywords").toUpper(), this);
#ifdef APP_MAC
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
        QLabel *itemLabel = new QLabel("<a href=\"" + keyword
                                       + "\" style=\"color:palette(text); text-decoration:none\">"
                                       + keyword + "</a>", this);

        itemLabel->setMaximumWidth(queryEdit->width() + watchButton->width());
        // itemLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        // Make links navigable with the keyboard too
        itemLabel->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard | Qt::LinksAccessibleByMouse);

        connect(itemLabel, SIGNAL(linkActivated(QString)), this, SLOT(watch(QString)));
        recentKeywordsLayout->addWidget(itemLabel);
    }

}



void SearchView::watch() {
    QString query = queryEdit->text().simplified();
    watch(query);
}

void SearchView::textChanged(const QString &text) {
    watchButton->setEnabled(!text.simplified().isEmpty());
}

void SearchView::watch(QString query) {

    // check for empty query
    if (query.length() == 0) {
        queryEdit->setFocus(Qt::OtherFocusReason);
        return;
    }

    // go!
    emit search(query);
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
#ifdef APP_MAC
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
