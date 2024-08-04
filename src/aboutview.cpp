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

#include "aboutview.h"
#include "constants.h"
#ifdef APP_EXTRA
#include "extra.h"
#endif
#ifdef APP_ACTIVATION
#include "activation.h"
#endif
#ifdef APP_MAC
#include "mac_startup.h"
#include "macutils.h"
#endif
#include "appwidget.h"
#include "clickablelabel.h"
#include "fontutils.h"
#include "iconutils.h"
#include "mainwindow.h"
#include "views.h"

#ifdef UPDATER
#include "updater.h"
#include "waitingspinnerwidget.h"
#endif

AboutView::AboutView(QWidget *parent) : QWidget(parent) {
    const int padding = 30;
    const char *buildYear = __DATE__ + 7;

    setBackgroundRole(QPalette::Base);
    setForegroundRole(QPalette::Text);
    setAutoFillBackground(true);

    QBoxLayout *verticalLayout = new QVBoxLayout(this);
    verticalLayout->setContentsMargins(0, 0, 0, 0);
    verticalLayout->setSpacing(0);

    QBoxLayout *aboutlayout = new QHBoxLayout();
    verticalLayout->addLayout(aboutlayout, 1);
    aboutlayout->setAlignment(Qt::AlignCenter);
    aboutlayout->setContentsMargins(padding, padding, padding, padding);
    aboutlayout->setSpacing(padding);

    ClickableLabel *logo = new ClickableLabel();
    auto setLogoPixmap = [logo] {
        logo->setPixmap(IconUtils::pixmap(":/images/app.png", logo->devicePixelRatioF()));
    };
    setLogoPixmap();
    connect(window()->windowHandle(), &QWindow::screenChanged, this, setLogoPixmap);

    connect(logo, &ClickableLabel::clicked, MainWindow::instance(), &MainWindow::visitSite);
    aboutlayout->addWidget(logo, 0, Qt::AlignTop);

    QBoxLayout *layout = new QVBoxLayout();
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(padding);
    layout->setContentsMargins(padding / 2, padding / 2, padding / 2, padding / 2);
    aboutlayout->addLayout(layout);

    QColor lightTextColor = palette().text().color();
#ifdef APP_MAC
    lightTextColor.setAlphaF(.75);
#endif
#ifdef APP_MAC
    QColor linkColor = mac::accentColor();
#else
    QColor linkColor = palette().highlight().color();
#endif

    QString info = "<html><style>"
                   "body { color: " +
                   lightTextColor.name(QColor::HexArgb) +
                   "; } "
                   "h1 { color: palette(text); font-weight: 100; } "
                   "a { color: " +
                   linkColor.name(QColor::HexArgb) +
                   "; text-decoration: none; font-weight: normal; }"
                   "</style><body>";

    info += "<h1>" + QString(Constants::NAME) +
            "</h1>"
            "<p>" +
            tr("There's life outside the browser!") +
            "</p>"
            "<p>" +
            tr("Version %1").arg(Constants::VERSION) + "</p>" +
            QString("<p><a href=\"%1/\">%1</a></p>").arg(Constants::WEBSITE);

#ifdef APP_ACTIVATION
    QString email = Activation::instance().getEmail();
    if (!email.isEmpty()) info += "<p>" + tr("Licensed to: %1").arg("<b>" + email + "</b>");
#endif

#ifndef APP_EXTRA
    info += "<p>" +
            tr("%1 is Free Software but its development takes precious time.")
                    .arg(Constants::NAME) +
            "<br/>" +
            tr("Please <a href='%1'>donate</a> to support the continued development of %2.")
                    .arg(QString(Constants::WEBSITE).append("#donate"), Constants::NAME) +
            "</p>";
#endif

    info += "<p>" +
            tr("Translate %1 to your native language using %2")
                    .arg(Constants::NAME)
                    .arg("<a href='https://app.transifex.com/flaviotordini/" +
                         QString(Constants::UNIX_NAME) + "/'>Transifex</a>") +
            "</p>";

    info += "<p>" +
            tr("Powered by %1")
                    .arg("<a href='https://" + QLatin1String(Constants::ORG_DOMAIN) +
                         "/opensource'>" + tr("Open-source software") + "</a>") +
            "</p>"

#ifndef APP_EXTRA
            "<p>" +
            tr("Released under the <a href='%1'>GNU General Public License</a>")
                    .arg("http://www.gnu.org/licenses/gpl.html") +
            "</p>"
#endif
            "<p>&copy; " +
            buildYear + " " + Constants::ORG_NAME +
            "</p>"
            "</body></html>";

    QLabel *infoLabel = new QLabel(info, this);
    infoLabel->setOpenExternalLinks(true);
    infoLabel->setWordWrap(true);
    layout->addWidget(infoLabel);

#ifdef UPDATER
    int capHeight = fontMetrics().capHeight();

    QBoxLayout *updateLayout = new QHBoxLayout();
    updateLayout->setContentsMargins(0, 0, 0, 0);
    updateLayout->setSpacing(capHeight);
    updateLayout->setAlignment(Qt::AlignLeft);

    auto spinner = new WaitingSpinnerWidget(this, false, false);
    spinner->setColor(palette().windowText().color());
    spinner->setLineLength(capHeight / 2);
    spinner->setNumberOfLines(spinner->lineLength() * 2);
    spinner->setInnerRadius(spinner->lineLength());
    auto spinnerStartStop = [spinner](auto status) {
        if (status == Updater::Status::DownloadingUpdate)
            spinner->start();
        else
            spinner->stop();
    };
    connect(&Updater::instance(), &Updater::statusChanged, this, spinnerStartStop);
    updateLayout->addWidget(spinner);
    spinnerStartStop(Updater::instance().getStatus());

    updateLayout->addWidget(Updater::instance().getLabel());

    layout->addLayout(updateLayout);
#endif

    QLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setAlignment(Qt::AlignLeft);

#ifdef UPDATER
    buttonLayout->addWidget(Updater::instance().getButton());
#endif

    auto closeButton = new QPushButton(tr("&Close"), this);
    closeButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    closeButton->setDefault(true);
    closeButton->setShortcut(Qt::Key_Escape);
    closeButton->setFocus();
    connect(closeButton, &QAbstractButton::clicked, MainWindow::instance()->getViews(),
            &Views::goBack);
    buttonLayout->addWidget(closeButton);

    layout->addLayout(buttonLayout);

#ifndef APP_WIN_STORE
    verticalLayout->addWidget(new AppsWidget(this));
#endif
}

void AboutView::showEvent(QShowEvent *event) {
#ifdef UPDATER
    Updater::instance().checkWithoutUI();
#endif
}
