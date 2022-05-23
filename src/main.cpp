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

#include <QtNetwork>
#include <QtWidgets>

#ifdef APP_MAC_STORE
typedef QApplication SingleApplication;
#else
#include <singleapplication.h>
#endif

#include "constants.h"
#include "iconutils.h"
#include "updateutils.h"

#include "mainwindow.h"
#include "searchparams.h"
#ifdef APP_EXTRA
#include "extra.h"
#endif
#ifdef Q_OS_MAC
#include "mac_startup.h"
#endif

void showWindow(SingleApplication &app, const QString &pkgDataDir) {
    MainWindow *mainWin = new MainWindow();

#ifndef APP_MAC
    QIcon appIcon;
    if (!pkgDataDir.isEmpty()) {
        appIcon = IconUtils::icon(Constants::UNIX_NAME);
    } else {
        QString dataDir = qApp->applicationDirPath() + "/data";
        const int iconSizes[] = {16, 22, 32, 48, 64, 128, 256, 512};
        for (int i = 0; i < 8; i++) {
            QString size = QString::number(iconSizes[i]);
            QString png = dataDir + '/' + size + 'x' + size + '/' + Constants::UNIX_NAME +
                          QLatin1String(".png");
            appIcon.addFile(png, QSize(iconSizes[i], iconSizes[i]));
        }
    }
    if (appIcon.isNull()) appIcon.addFile(":/images/app.png");
    mainWin->setWindowIcon(appIcon);
#endif
#ifndef APP_MAC_STORE
    mainWin->connect(&app, &SingleApplication::receivedMessage, mainWin,
                     [mainWin](auto instanceId, auto message) {
                         Q_UNUSED(instanceId);
                         mainWin->messageReceived(message);
                     });
#endif

    mainWin->show();
}

int main(int argc, char **argv) {
#ifndef QT_NO_DEBUG_OUTPUT
    qSetMessagePattern("[%{function}] %{message}");
#endif

#ifdef MEDIA_MPV
    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
#ifdef APP_MAC
    format.setMajorVersion(4);
    format.setMinorVersion(1);
#endif
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);
#endif

#ifdef Q_OS_MAC
    mac::MacMain();
#endif

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    SingleApplication app(argc, argv);
    QString message;
    if (app.arguments().size() > 1) {
        message = app.arguments().at(1);
        if (message == QLatin1String("--help")) {
            MainWindow::printHelp();
            return 0;
        }
    }
    if (app.sendMessage(message.toUtf8())) return 0;

    app.setApplicationName(Constants::NAME);
    app.setOrganizationName(Constants::ORG_NAME);
    app.setOrganizationDomain(Constants::ORG_DOMAIN);
    app.setApplicationVersion(Constants::VERSION);
    app.setAttribute(Qt::AA_DontShowIconsInMenus);
    app.setWheelScrollLines(1);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);

#ifdef APP_EXTRA
    Extra::appSetup(&app);
#else
    QFile cssFile(":/style.css");
    cssFile.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(cssFile.readAll());
    app.setStyleSheet(styleSheet);
    cssFile.close();
#endif

    // qt translations
    QTranslator qtTranslator;
    qtTranslator.load(QLatin1String("qt_") + QLocale::system().name(),
                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);

    QString pkgDataDir;
#ifdef PKGDATADIR
    pkgDataDir = QLatin1String(PKGDATADIR);
#endif

    // app translations
#ifdef APP_MAC
    QString localeDir = qApp->applicationDirPath() + QLatin1String("/../Resources/locale");
#else
    QString localeDir = qApp->applicationDirPath() + QLatin1String("/locale");
#endif
    if (!QDir(localeDir).exists()) {
        localeDir = pkgDataDir + QLatin1String("/locale");
    }
    qDebug() << "Using locale dir" << localeDir << QLocale::system();
    QTranslator translator;
    translator.load(QLocale::system(), QString(), QString(), localeDir);
    app.installTranslator(&translator);

    QNetworkProxyFactory::setUseSystemConfiguration(true);

    IconUtils::setSizes({16, 24, 32, 88, 128});

    UpdateUtils::init();

    showWindow(app, pkgDataDir);

    return app.exec();
}
