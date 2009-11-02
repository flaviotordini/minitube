#include <QtGui>
#include <qtsingleapplication.h>
#include "Constants.h"
#include "MainWindow.h"

int main(int argc, char **argv) {

    QtSingleApplication app(argc, argv);
    if (app.sendMessage("Wake up!"))
        return 0;

    app.setApplicationName(Constants::APP_NAME);
    app.setOrganizationName(Constants::ORG_NAME);
    app.setOrganizationDomain(Constants::ORG_DOMAIN);

    QString locale = QLocale::system().name();

    // qt translations
    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(),
                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);

    // app translations
    QString dataDir = QLatin1String(PKGDATADIR);
    QString localeDir = dataDir + QDir::separator() + "locale";
    // if app was not "installed" use the app directory
    if (!QFile::exists(localeDir)) {
        localeDir = qApp->applicationDirPath() + QDir::separator() + "locale";
        // qDebug() << "Using locale dir" << localeDir << locale;
    }
    QTranslator translator;
    translator.load(locale, localeDir);
    app.installTranslator(&translator);
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));

    MainWindow mainWin;
    mainWin.setWindowTitle(Constants::APP_NAME);

    if (!QFile::exists(dataDir)) {
        dataDir = qApp->applicationDirPath() + "/data";
    }
    const int iconSizes [] = { 16, 22, 24, 32, 48, 64, 128, 256 };
    QIcon appIcon;
    for (int i = 0; i < 8; i++) {
        QString size = QString::number(iconSizes[i]);
        QString png = dataDir + "/" + size + "x" + size + "/minitube.png";
        // qDebug() << png;
        appIcon.addFile(png);
    }
    mainWin.setWindowIcon(appIcon);

    mainWin.show();

    app.setActivationWindow(&mainWin, true);

    // all string literals are UTF-8
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    return app.exec();
}
