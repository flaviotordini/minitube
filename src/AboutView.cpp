#include "AboutView.h"
#include "constants.h"

AboutView::AboutView(QWidget *parent) : QWidget(parent) {

    QBoxLayout *aboutlayout = new QHBoxLayout(this);
    aboutlayout->setAlignment(Qt::AlignCenter);
    aboutlayout->setMargin(30);
    aboutlayout->setSpacing(30);

    QLabel *logo = new QLabel(this);
    logo->setPixmap(QPixmap(":/images/app.png"));
    aboutlayout->addWidget(logo, 0, Qt::AlignTop);

    QBoxLayout *layout = new QVBoxLayout();
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(30);
    aboutlayout->addLayout(layout);

    QString info = "<html><style>a { color: palette(text); text-decoration: none; font-weight: bold }</style><body><h1>" + QString(Constants::NAME) + "</h1>"
            "<p>" + tr("There's life outside the browser!") + "</p>"
            "<p>" + tr("Version %1").arg(Constants::VERSION) + "</p>"
            + QString("<p><a href=\"%1/\">%1</a></p>").arg(Constants::WEBSITE) +

        #if !defined(APP_MAC) && !defined(APP_WIN)
            "<p>" +  tr("%1 is Free Software but its development takes precious time.").arg(Constants::NAME) + "<br/>"
            + tr("Please <a href='%1'>donate</a> to support the continued development of %2.")
            .arg(QString(Constants::WEBSITE).append("#donate"), Constants::NAME) + "</p>"
        #endif

            "<p>" + tr("You may want to try my other apps as well:") + "</p>"
            + "<ul>"

            + "<li>" + tr("%1, a YouTube music player")
            .arg("<a href='http://flavio.tordini.org/musictube'>Musictube</a>")
            + "</li>"

            + "<li>" + tr("%1, a music player")
            .arg("<a href='http://flavio.tordini.org/musique'>Musique</a>")
            + "</li>"

            + "</ul>"

            "<p>" + tr("Translate %1 to your native language using %2").arg(Constants::NAME)
            .arg("<a href='http://www.transifex.net/projects/p/" + QString(Constants::UNIX_NAME) + "/resource/main/'>Transifex</a>")
            + "</p>"

            "<p>"
            + tr("Icon designed by %1.").arg("<a href='http://www.kolorguild.com/'>David Nel</a>")
            + "</p>"

        #if !defined(APP_MAC) && !defined(APP_WIN)
            "<p>" + tr("Released under the <a href='%1'>GNU General Public License</a>")
            .arg("http://www.gnu.org/licenses/gpl.html") + "</p>"
        #endif
            "<p>&copy; 2009-2011 " + Constants::ORG_NAME + "</p>"
            "</body></html>";
    QLabel *infoLabel = new QLabel(info, this);
    infoLabel->setOpenExternalLinks(true);
    infoLabel->setWordWrap(true);
    layout->addWidget(infoLabel);

    QLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setAlignment(Qt::AlignLeft);
    QPushButton *closeButton = new QPushButton(tr("&Close"), this);
    closeButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    closeButton->setDefault(true);
    closeButton->setFocus(Qt::OtherFocusReason);
    connect(closeButton, SIGNAL(clicked()), parent, SLOT(goBack()));
    buttonLayout->addWidget(closeButton);

    layout->addLayout(buttonLayout);

}

void AboutView::paintEvent(QPaintEvent * /*event*/) {
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
