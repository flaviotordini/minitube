#include <QtGui>
#include <qtsingleapplication.h>
#include "constants.h"
#include "MainWindow.h"
#ifdef APP_MAC
// #include "local/mac/mac_startup.h"
#endif
#ifdef APP_WIN
#include "local/win/qtwin.h"
#endif

int main(int argc, char **argv) {

#ifdef APP_MAC
    // mac::MacMain();
#endif

    QtSingleApplication app(argc, argv);
    if (app.sendMessage("Wake up!"))
        return 0;

    app.setApplicationName(Constants::APP_NAME);
    app.setOrganizationName(Constants::ORG_NAME);
    app.setOrganizationDomain(Constants::ORG_DOMAIN);
#ifndef APP_MAC
    app.setWheelScrollLines(1);
#endif

    const QString locale = QLocale::system().name();

    // qt translations
    QTranslator qtTranslator;
    qtTranslator.load("qt_" + locale,
                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);

    // app translations
#ifdef PKGDATADIR
    QString dataDir = QLatin1String(PKGDATADIR);
#else
    QString dataDir = "";
#endif
    QString localeDir = qApp->applicationDirPath() + QDir::separator() + "locale";
    if (!QFile::exists(localeDir)) {
        localeDir = dataDir + QDir::separator() + "locale";
    }
    // qDebug() << "Using locale dir" << localeDir << locale;
    QTranslator translator;
    translator.load(locale, localeDir);
    app.installTranslator(&translator);
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));

    MainWindow mainWin;
    mainWin.setWindowTitle(Constants::APP_NAME);

// no window icon on Mac
#ifndef APP_MAC
    if (!QFile::exists(dataDir)) {
        dataDir = qApp->applicationDirPath() + "/data";
    }
    const int iconSizes [] = { 16, 22, 24, 32, 48, 64, 128, 256 };
    QIcon appIcon;
    for (int i = 0; i < 8; i++) {
        QString size = QString::number(iconSizes[i]);
        QString png = dataDir + "/" + size + "x" + size + "/minitube.png";
        // qDebug() << png;
        appIcon.addFile(png, QSize(iconSizes[i], iconSizes[i]));
    }
    mainWin.setWindowIcon(appIcon);
#endif

#ifdef APP_WIN
    if (QtWin::isCompositionEnabled()) {
        QtWin::extendFrameIntoClientArea(&mainWin);
        mainWin.setContentsMargins(0, 0, 0, 0);
    }
    app.setFont(QFont("Segoe UI", 9));
#endif

    mainWin.show();

    app.setActivationWindow(&mainWin, true);

    // all string literals are UTF-8
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    return app.exec();
}
