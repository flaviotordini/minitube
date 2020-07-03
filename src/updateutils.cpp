#include "updateutils.h"

#include <QtCore>

#include "constants.h"
#include "iconutils.h"
#include "mainwindow.h"

#ifdef UPDATER
#include "updater.h"
#ifdef UPDATER_SPARKLE
#include "sparkleupdater.h"
#else
#include "defaultupdater.h"
#endif
#endif

namespace UpdateUtils {

void init() {
#ifdef UPDATER

#ifdef UPDATER_SPARKLE
    Updater::setInstance(new updater::SparkleUpdater());
#else
    auto updater = new updater::DefaultUpdater();

    // updater->setAppName(Constants::NAME);
    // updater->setIcon(IconUtils::pixmap(":/images/app.png", qApp->devicePixelRatio()));
    // updater->setLocalVersion(Constants::VERSION);

    QUrl url(QLatin1String(Constants::WEBSITE) + "-ws/release.xml");
    QUrlQuery q;
    q.addQueryItem("v", Constants::VERSION);
#ifdef APP_MAC
    q.addQueryItem("os", "mac");
#endif
#ifdef APP_WIN
    q.addQueryItem("os", "win");
#endif
#ifdef APP_MAC_STORE
    q.addQueryItem("store", "mac");
#endif
    url.setQuery(q);
    updater->setManifestUrl(url);

    QString ext = Constants::UNIX_NAME;
#ifdef APP_MAC
    ext = ".dmg";
#elif APP_WIN
    ext = ".exe";
#else
    ext = ".deb";
#endif
    updater->setDownloadUrl(QUrl("https://" + QLatin1String(Constants::ORG_DOMAIN) + "/files/" +
                                 Constants::UNIX_NAME + "/" + Constants::UNIX_NAME + ext));

    Updater::setInstance(updater);
#endif

#endif
}

} // namespace UpdateUtils
