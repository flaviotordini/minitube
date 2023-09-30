#include "subscriptionimportview.h"
#include "fontutils.h"
#include "iconutils.h"
#include "ytchannel.h"

#include "homeview.h"
#include "mainwindow.h"
#include "views.h"

QAction *SubscriptionImportView::buildAction(QWidget *parent) {
    auto a = new QAction("Import subscriptions...", parent);
    a->setMenuRole(QAction::ApplicationSpecificRole);
    connect(a, &QAction::triggered, parent, [parent] {
        auto dialog = new SubscriptionImportView(parent);
        dialog->show();
    });
    return a;
}

SubscriptionImportView::SubscriptionImportView(QWidget *parent) : QWidget(parent) {
    auto layout = new QVBoxLayout(this);
    layout->setSpacing(fontMetrics().xHeight() * 4);
    layout->setAlignment(Qt::AlignCenter);

    auto closeButton = new QToolButton();
    closeButton->setIcon(IconUtils::icon("close"));
    closeButton->setIconSize({32, 32});
    closeButton->setShortcut(Qt::Key_Escape);
    closeButton->setStyleSheet("border:0");
    connect(closeButton, &QToolButton::clicked, this,
            [] { MainWindow::instance()->getViews()->goBack(); });
    layout->addWidget(closeButton, 0, Qt::AlignRight);

    auto icon = new QLabel();
    icon->setPixmap(IconUtils::icon("import").pixmap(128, 128));
    layout->addWidget(icon, 0, Qt::AlignHCenter);

    QString url = "https://takeout.google.com/takeout/custom/youtube";
    QString msg =
            "Follow these steps to import your YouTube subscriptions:<ul>"
            "<li style='line-height:1.5'>Visit <a href='%1'>%2</a>"
            "<li style='line-height:1.5'>Click <i>All YouTube data included</i>, deselect all "
            "and select only subscriptions"
            "<li style='line-height:1.5'>Download the zip file and extract it";
    msg = msg.arg(url, "Google Takeout");
    auto tip = new QLabel(msg);
    tip->setOpenExternalLinks(true);
    tip->setFont(FontUtils::medium());
    layout->addWidget(tip);

    auto button = new QPushButton("Open subscriptions.csv");
    button->setDefault(true);
    button->setFocus();
    button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(button, &QPushButton::clicked, this, [this] {
        auto dir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open subscriptions.csv"), dir,
                                                        tr("YouTube data (*.csv *.json)"));
        if (!fileName.isEmpty()) {
            auto w = MainWindow::instance();
            QString msg;
            QFile file(fileName);
            if (file.open(QFile::ReadOnly)) {
                int count = 0;
                if (QFileInfo(fileName).suffix().toLower() == "csv") {
                    int lineNumber = 1;
                    while (!file.atEnd()) {
                        QByteArray line = file.readLine();
                        if (lineNumber > 1) {
                            auto fields = line.split(',');
                            auto id = fields.first();
                            qDebug() << "Subscribing to" << id;
                            if (YTChannel::subscribe(id)) count++;
                        }
                        lineNumber++;
                    }
                } else {
                    const auto array = QJsonDocument::fromJson(file.readAll()).array();
                    for (const auto &v : array) {
                        auto i = v.toObject();
                        auto id = i["snippet"]
                                          .toObject()["resourceId"]
                                          .toObject()["channelId"]
                                          .toString();
                        qDebug() << "Subscribing to" << id;
                        if (YTChannel::subscribe(id)) count++;
                    }
                }
                msg = tr("Subscribed to %n channel(s)", "", count);
                w->showHome();
                w->getHomeView()->showChannels();
            } else
                msg = tr("Cannot open file");
            if (!msg.isEmpty()) QTimer::singleShot(0, w, [w, msg] { w->showMessage(msg); });
        }
    });
    layout->addWidget(button, 0, Qt::AlignHCenter);
}

void SubscriptionImportView::showEvent(QShowEvent *event) {}
