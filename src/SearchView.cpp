#include "SearchView.h"
#include "Constants.h"

static const QString recentKeywordsKey = "recentKeywords";
static const int PADDING = 30;

SearchView::SearchView(QWidget *parent) : QWidget(parent) {

#ifdef Q_WS_MAC
    // speedup painting since we'll paint the whole background
    // by ourselves anyway in paintEvent()
    setAttribute(Qt::WA_OpaquePaintEvent);
#endif

    QFont biggerFont;
    biggerFont.setPointSize(biggerFont.pointSize()*1.5);

    QFont smallerFont;
    smallerFont.setPointSize(smallerFont.pointSize()*.85);
    smallerFont.setBold(true);

    QBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setMargin(0);
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
            new QLabel("<h1>" +
                       tr("Welcome to <a href='%1'>%2</a>,")
                       .replace("<a ", "<a style='color:palette(text)'")
                       .arg(Constants::WEBSITE, Constants::APP_NAME)
                       + "</h1>", this);
    welcomeLabel->setOpenExternalLinks(true);
    layout->addWidget(welcomeLabel);

    layout->addSpacing(PADDING);

    QLabel *tipLabel = new QLabel(tr("Enter a keyword to start watching videos."), this);
    tipLabel->setFont(biggerFont);
    layout->addWidget(tipLabel);

    layout->addSpacing(10);

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

    layout->addSpacing(PADDING);

    QHBoxLayout *otherLayout = new QHBoxLayout();

    recentKeywordsLayout = new QVBoxLayout();
    recentKeywordsLayout->setSpacing(5);
    recentKeywordsLayout->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    recentKeywordsLabel = new QLabel(tr("Recent keywords").toUpper(), this);
    recentKeywordsLabel->hide();
    recentKeywordsLabel->setForegroundRole(QPalette::Dark);
    recentKeywordsLabel->setFont(smallerFont);
    recentKeywordsLayout->addWidget(recentKeywordsLabel);

    otherLayout->addLayout(recentKeywordsLayout);

    layout->addLayout(otherLayout);

    mainLayout->addStretch();

    setLayout(mainLayout);

    updateChecker = 0;
    checkForUpdate();
}

void SearchView::paintEvent(QPaintEvent * /*event*/) {

#ifdef Q_WS_MAC
    QPainter painter(this);
    QLinearGradient linearGrad(0, 0, 0, height());
    QPalette palette;
    linearGrad.setColorAt(0, palette.color(QPalette::Light));
    linearGrad.setColorAt(1, palette.color(QPalette::Midlight));
    painter.fillRect(0, 0, width(), height(), QBrush(linearGrad));
#endif

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
    foreach (QString keyword, keywords) {
        QLabel *itemLabel = new QLabel("<a href=\"" + keyword
                                       + "\" style=\"color:palette(text); text-decoration:none\">"
                                       + keyword + "</a>", this);

        itemLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
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
            .arg(
                    Constants::APP_NAME,
                    QString(Constants::WEBSITE).append("#download"),
                    version)
            );
    message->setOpenExternalLinks(true);
    message->setMargin(10);
    message->setBackgroundRole(QPalette::ToolTipBase);
    message->setForegroundRole(QPalette::ToolTipText);
    message->setAutoFillBackground(true);
    message->show();
    if (updateChecker) delete updateChecker;
}
