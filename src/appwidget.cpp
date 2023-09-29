#include "appwidget.h"
#include "constants.h"
#include "http.h"
#ifdef APP_EXTRA
#include "updatedialog.h"
#endif

AppsWidget::AppsWidget(QWidget *parent) : QWidget(parent) {
    const int padding = 30;

    QBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(padding, padding, padding, padding);
    layout->setSpacing(padding*2);
    layout->setAlignment(Qt::AlignCenter);

#ifdef APP_MAC
    const QString ext = "dmg";
#elif defined APP_WIN
    const QString ext = "exe";
#else
    const QString ext = "deb";
#endif

#ifdef APP_MAC
    setupApp("Sofa", "sofa." + ext);
#endif
    setupApp("Finetune", "finetune." + ext);
    setupApp("Musictube", "musictube." + ext);
    setupApp("Musique", "musique." + ext);
}

void AppsWidget::setupApp(const QString &name, const QString &code) {
    AppWidget *w = new AppWidget(name, code);
    layout()->addWidget(w);
}

void AppsWidget::paintEvent(QPaintEvent *e) {
    Q_UNUSED(e);
    QStyleOption o;
    o.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &o, &p, this);
}

AppWidget::AppWidget(const QString &name, const QString &code, QWidget *parent)
    : QWidget(parent), name(name), downloadButton(nullptr) {
    const QString unixName = code.left(code.lastIndexOf('.'));
    const QString baseUrl = QLatin1String("https://") + Constants::ORG_DOMAIN;
    const QString filesUrl = baseUrl + QLatin1String("/files/");
    url = filesUrl + unixName + QLatin1String("/") + code;
    webPage = baseUrl + QLatin1String("/") +  unixName;

    QBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignHCenter);

    icon = new QLabel();
    icon->setMinimumHeight(128);
    layout->addWidget(icon);
    QString pixelRatioString;
    if (devicePixelRatioF() > 1.0)
        pixelRatioString = '@' + QString::number(devicePixelRatio()) + 'x';
    const QString iconUrl = filesUrl + QLatin1String("products/") + unixName + pixelRatioString +
                            QLatin1String(".png");
    QObject *reply = Http::instance().get(iconUrl);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(iconDownloaded(QByteArray)));

    QLabel *appTitle = new QLabel(name);
    appTitle->setAlignment(Qt::AlignHCenter);
    layout->addWidget(appTitle);

#ifdef APP_EXTRA
#if !defined(APP_UBUNTU) && !defined(APP_MAC_STORE)
    downloadButton = new QPushButton(tr("Download"));
    downloadButton->setAttribute(Qt::WA_MacSmallSize);
    downloadButton->setCursor(Qt::ArrowCursor);
    QSizePolicy sp = downloadButton->sizePolicy();
    sp.setHorizontalPolicy(QSizePolicy::Fixed);
    sp.setRetainSizeWhenHidden(true);
    downloadButton->setSizePolicy(sp);
    connect(downloadButton, SIGNAL(clicked(bool)), SLOT(downloadApp()));
    layout->addWidget(downloadButton, Qt::AlignHCenter);
    layout->setAlignment(downloadButton, Qt::AlignHCenter);
    downloadButton->hide();
#endif
#endif

    setCursor(Qt::PointingHandCursor);
}

void AppWidget::enterEvent(CompatibleEnterEvent *e) {
    Q_UNUSED(e);
    if (downloadButton) downloadButton->show();
}

void AppWidget::leaveEvent(QEvent *e) {
    Q_UNUSED(e);
    if (downloadButton) downloadButton->hide();
}

void AppWidget::mouseReleaseEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton) {
        QDesktopServices::openUrl(webPage);
    }
}

void AppWidget::iconDownloaded(const QByteArray &bytes) {
    QPixmap pixmap;
    pixmap.loadFromData(bytes, "PNG");
    pixmap.setDevicePixelRatio(devicePixelRatioF());
    icon->setPixmap(pixmap);
}

void AppWidget::downloadApp() {
#ifdef APP_EXTRA
    if (!icon) return;
    UpdateDialog *dialog =
            new UpdateDialog(icon->pixmap(Qt::ReturnByValue), name, QString(), url, this);
    dialog->downloadUpdate();
    dialog->show();
#endif
}
