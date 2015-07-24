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
#include "macutils.h"
#include "mac_startup.h"
#endif

AboutView::AboutView(QWidget *parent) : QWidget(parent) {

    QBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->setAlignment(Qt::AlignCenter);
    hLayout->setMargin(30);
    hLayout->setSpacing(30);

    QLabel *logo = new QLabel(this);
    logo->setPixmap(QPixmap(":/images/app.png"));
    hLayout->addWidget(logo, 0, Qt::AlignTop);

    QBoxLayout *layout = new QVBoxLayout();
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(30);
    hLayout->addLayout(layout);

    QString info = "<html><style>a { color: palette(text); text-decoration: none; font-weight: bold }</style><body>"
            "<h1 style='font-weight:normal'>" + QString(Constants::NAME) + "</h1>"
            "<p>" + tr("There's life outside the browser!") + "</p>"
            "<p>" + tr("Version %1").arg(Constants::VERSION) + "</p>"
            + QString("<p><a href=\"%1/\">%1</a></p>").arg(Constants::WEBSITE);

#ifdef APP_ACTIVATION
    if (Activation::instance().isActivated())
        info += "<p>" + tr("Licensed to: %1").arg("<b>" + Activation::instance().getEmail() + "</b>");
#endif

#ifndef APP_EXTRA
    info += "<p>" +  tr("%1 is Free Software but its development takes precious time.").arg(Constants::NAME) + "<br/>"
            + tr("Please <a href='%1'>donate</a> to support the continued development of %2.")
            .arg(QString(Constants::WEBSITE).append("#donate"), Constants::NAME) + "</p>";
#endif

    info += "<p>" + tr("You may want to try my other apps as well:") + "</p>"
            "<ul>"

            "<li>" + tr("%1, a YouTube music player")
            .arg("<a href='http://flavio.tordini.org/musictube'>Musictube</a>")
            + "</li>"

            "<li>" + tr("%1, a music player")
            .arg("<a href='http://flavio.tordini.org/musique'>Musique</a>")
            + "</li>"

            "</ul>"

            "<p>" + tr("Translate %1 to your native language using %2").arg(Constants::NAME)
            .arg("<a href='http://www.transifex.net/projects/p/" + QString(Constants::UNIX_NAME) + "/'>Transifex</a>")
            + "</p>"

            "<p>"
            + tr("Icon designed by %1.").arg("<a href='http://www.kolorguild.co.za/'>David Nel</a>")
            + "</p>"

        #ifndef APP_EXTRA
            "<p>" + tr("Released under the <a href='%1'>GNU General Public License</a>")
            .arg("http://www.gnu.org/licenses/gpl.html") + "</p>"
        #endif
            "<p>&copy; 2009-2015 " + Constants::ORG_NAME + "</p>"
            "</body></html>";
    QLabel *infoLabel = new QLabel(info, this);
    infoLabel->setOpenExternalLinks(true);
    infoLabel->setWordWrap(true);
    layout->addWidget(infoLabel);

    QLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setMargin(0);
    buttonLayout->setSpacing(0);
    buttonLayout->setAlignment(Qt::AlignLeft);

    closeButton = new QPushButton(tr("&Close"));
    closeButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    closeButton->setDefault(true);
    connect(closeButton, SIGNAL(clicked()), parent, SLOT(goBack()));
    buttonLayout->addWidget(closeButton);

    layout->addLayout(buttonLayout);
}

void AboutView::appear() {
#ifdef APP_MAC
    mac::uncloseWindow(window()->winId());
#ifdef APP_ACTIVATION
    mac::CheckForUpdates();
#endif
#endif
    closeButton->setFocus();
}
