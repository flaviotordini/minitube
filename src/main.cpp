#include <QtGui>
#include <qtsingleapplication.h>
#include "constants.h"
#include "mainwindow.h"
#include "searchparams.h"
#include "utils.h"
#ifndef Q_WS_X11
#include "extra.h"
#endif
#ifdef Q_WS_MAC
#include "mac_startup.h"
#endif

#ifdef Q_WS_X11
QString getThemeName() {
    QString themeName;

    QProcess process;
    process.start("dconf",
                  QStringList() << "read" << "/org/gnome/desktop/interface/gtk-theme");
    if (process.waitForFinished()) {
        themeName = process.readAllStandardOutput();
        themeName = themeName.trimmed();
        themeName.remove('\'');
        if (!themeName.isEmpty()) return themeName;
    }

    QString rcPaths = QString::fromLocal8Bit(qgetenv("GTK2_RC_FILES"));
    if (!rcPaths.isEmpty()) {
        QStringList paths = rcPaths.split(QLatin1String(":"));
        foreach (const QString &rcPath, paths) {
            if (!rcPath.isEmpty()) {
                QFile rcFile(rcPath);
                if (rcFile.exists() && rcFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QTextStream in(&rcFile);
                    while(!in.atEnd()) {
                        QString line = in.readLine();
                        if (line.contains(QLatin1String("gtk-theme-name"))) {
                            line = line.right(line.length() - line.indexOf(QLatin1Char('=')) - 1);
                            line.remove(QLatin1Char('\"'));
                            line = line.trimmed();
                            themeName = line;
                            break;
                        }
                    }
                }
            }
            if (!themeName.isEmpty())
                break;
        }
    }

    // Fall back to gconf
    if (themeName.isEmpty())
        themeName = QGtkStyle::getGConfString(QLatin1String("/desktop/gnome/interface/gtk_theme"));

    return themeName;
}
#endif

int main(int argc, char **argv) {

#ifdef Q_WS_MAC
    mac::MacMain();
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
    app.setWheelScrollLines(1);
    app.setAttribute(Qt::AA_DontShowIconsInMenus);

#ifndef Q_WS_X11
    Extra::appSetup(&app);
#else
    bool isGtk = app.style()->metaObject()->className() == QLatin1String("QGtkStyle");
    if (isGtk) {
        app.setProperty("gtk", isGtk);
        QString themeName = getThemeName();
        app.setProperty("style", themeName);
    }
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
    QString localeDir = qApp->applicationDirPath() + "/locale";
    if (!QDir(localeDir).exists()) {
        localeDir = dataDir + "/locale";
    }
    // qDebug() << "Using locale dir" << localeDir << locale;
    QTranslator translator;
    translator.load(QLocale::system(), QString(), QString(), localeDir);
    app.installTranslator(&translator);
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));

    MainWindow mainWin;
    mainWin.setWindowTitle(Constants::NAME);

#ifndef Q_WS_X11
    Extra::windowSetup(&mainWin);
#else
    mainWin.setProperty("style", app.property("style"));
#endif

// no window icon on Mac
#ifndef APP_MAC
    QIcon appIcon;
    if (QDir(dataDir).exists()) {
        appIcon = Utils::icon(Constants::UNIX_NAME);
    } else {
        dataDir = qApp->applicationDirPath() + "/data";
        const int iconSizes [] = { 16, 22, 32, 48, 64, 128, 256, 512 };
        for (int i = 0; i < 8; i++) {
            QString size = QString::number(iconSizes[i]);
            QString png = dataDir + "/" + size + "x" + size + "/" + Constants::UNIX_NAME + ".png";
            appIcon.addFile(png, QSize(iconSizes[i], iconSizes[i]));
        }
    }
    if (appIcon.isNull()) {
        appIcon.addFile(":/images/app.png");
    }
    mainWin.setWindowIcon(appIcon);
#endif

    mainWin.connect(&app, SIGNAL(messageReceived(const QString &)), &mainWin, SLOT(messageReceived(const QString &)));
    app.setActivationWindow(&mainWin, true);

    // all string literals are UTF-8
    // QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    if (app.arguments().size() > 1) {
        QString query = app.arguments().at(1);
        if (query.startsWith(QLatin1String("--"))) {
            mainWin.messageReceived(query);
            return 0;
        } else {
            SearchParams *searchParams = new SearchParams();
            searchParams->setKeywords(query);
            mainWin.showMedia(searchParams);
        }
    }

    mainWin.show();

    // Seed random number generator
    qsrand(QDateTime::currentDateTime().toTime_t());

    return app.exec();
}
