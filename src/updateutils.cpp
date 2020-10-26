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
#include "runinstaller.h"
#include "simplexmlparser.h"
#endif
#endif

namespace UpdateUtils {

void init() {
#ifdef UPDATER

#ifdef UPDATER_SPARKLE
    Updater::setInstance(new updater::SparkleUpdater());
#else
    auto updater = new updater::DefaultUpdater();

    QUrl manifestUrl(QLatin1String(Constants::WEBSITE) + "-ws/release.xml");
    updater->setManifestUrl(manifestUrl);
    updater->setParser(new updater::SimpleXmlParser());

    QString ext;
#ifdef APP_MAC
    ext = ".dmg";
#elif defined APP_WIN
    ext = ".exe";
#else
    ext = ".deb";
#endif
    QUrl downloadUrl("https://" + QLatin1String(Constants::ORG_DOMAIN) + "/files/" +
                     Constants::UNIX_NAME + "/" + Constants::UNIX_NAME + ext);
    updater->setDownloadUrl(downloadUrl);

    auto installer = new updater::RunInstaller;
#ifdef APP_WIN
    installer->setArguments({"/S"});
    installer->setRelaunchArguments({"/relaunch"});
#endif
#ifdef APP_LINUX
    installer->setCommand({"dpkg"});
    installer->setArguments({"-i", "%filename%"});
    installer->setRunAsAdmin(true);
    installer->setAutoRestart(true);
#endif
    updater->setInstaller(installer);

    Updater::setInstance(updater);
#endif

#endif
}

} // namespace UpdateUtils
