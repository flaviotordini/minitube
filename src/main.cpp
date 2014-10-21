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

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif
#include <qtsingleapplication.h>
#include "constants.h"
#include "mainwindow.h"
#include "searchparams.h"
#include "iconutils.h"
#ifdef APP_EXTRA
#include "extra.h"
#endif
#ifdef Q_OS_MAC
#include "mac_startup.h"
#endif

int main(int argc, char **argv) {

#ifdef Q_OS_MAC
    mac::MacMain();
    QFont::insertSubstitution(".Helvetica Neue DeskInterface", "Helvetica Neue");
#endif

    QtSingleApplication app(argc, argv);
    QString message = app.arguments().size() > 1 ? app.arguments().at(1) : QString();
    if (message == QLatin1String("--help")) {
        MainWindow::printHelp();
        return 0;
    }
    if (app.sendMessage(message))
        return 0;

    app.setApplicationName(QLatin1String(Constants::NAME));
    app.setOrganizationName(QLatin1String(Constants::ORG_NAME));
    app.setOrganizationDomain(QLatin1String(Constants::ORG_DOMAIN));
    app.setAttribute(Qt::AA_DontShowIconsInMenus);

#ifdef APP_EXTRA
    Extra::appSetup(&app);
#else
    QFile cssFile(":/style.css");
    cssFile.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(cssFile.readAll());
    app.setStyleSheet(styleSheet);
#endif

    // qt translations
    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(),
                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);

    // app translations
#ifdef PKGDATADIR
    QString dataDir = QLatin1String(PKGDATADIR);
#else
    QString dataDir = "";
#endif
#ifdef APP_MAC
    QString localeDir = qApp->applicationDirPath() + "/../Resources/locale";
#else
    QString localeDir = qApp->applicationDirPath() + "/locale";
#endif
    if (!QDir(localeDir).exists()) {
        localeDir = dataDir + "/locale";
    }
    // qDebug() << "Using locale dir" << localeDir << locale;
    QTranslator translator;
    translator.load(QLocale::system(), QString(), QString(), localeDir);
    app.installTranslator(&translator);
#if QT_VERSION < 0x050000
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));
#endif

    MainWindow mainWin;
    mainWin.show();

    // no window icon on Mac
#ifndef APP_MAC
    QIcon appIcon;
    if (QDir(dataDir).exists()) {
        appIcon = IconUtils::icon(Constants::UNIX_NAME);
    } else {
        dataDir = qApp->applicationDirPath() + "/data";
        const int iconSizes [] = { 16, 22, 32, 48, 64, 128, 256, 512 };
        for (int i = 0; i < 8; i++) {
            QString size = QString::number(iconSizes[i]);
            QString png = dataDir + "/" + size + "x" + size + "/" +
                    Constants::UNIX_NAME + ".png";
            appIcon.addFile(png, QSize(iconSizes[i], iconSizes[i]));
        }
    }
    if (appIcon.isNull()) {
        appIcon.addFile(":/images/app.png");
    }
    mainWin.setWindowIcon(appIcon);
#endif

    mainWin.connect(&app, SIGNAL(messageReceived(const QString &)),
                    &mainWin, SLOT(messageReceived(const QString &)));
    app.setActivationWindow(&mainWin, true);

    // all string literals are UTF-8
    // QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    // Seed random number generator
    qsrand(QDateTime::currentDateTime().toTime_t());

    return app.exec();
}
