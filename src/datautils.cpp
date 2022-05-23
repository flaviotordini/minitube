#include "datautils.h"

QString DataUtils::stringToFilename(const QString &s) {
    QString f = s;
    f.replace('(', '[');
    f.replace(')', ']');
    f.replace('/', ' ');
    f.replace('\\', ' ');
    f.replace('<', ' ');
    f.replace('>', ' ');
    f.replace(':', ' ');
    f.replace('"', ' ');
    f.replace('|', ' ');
    f.replace('?', ' ');
    f.replace('*', ' ');
    f = f.simplified();

    if (!f.isEmpty() && f.at(0) == '.') f = f.mid(1).trimmed();

    return f;
}

QString DataUtils::regioneCode(const QLocale &locale) {
    QString name = locale.name();
    int index = name.indexOf('_');
    if (index == -1) return QString();
    return name.right(index);
}

QString DataUtils::systemRegioneCode() {
    return regioneCode(QLocale::system());
}

uint DataUtils::parseIsoPeriod(const QString &isoPeriod) {
    int days = 0, hours = 0, minutes = 0, seconds = 0;

    const int len = isoPeriod.length();
    int digitStart = -1;
    for (int i = 0; i < len; ++i) {
        const QChar c = isoPeriod.at(i);
        if (c.isDigit()) {
            if (digitStart == -1) digitStart = i;
        } else if (digitStart != -1) {
            auto periodView = QStringView(isoPeriod).mid(digitStart, i - digitStart);
            if (c == 'H') {
                hours = periodView.toInt();
            } else if (c == 'M') {
                minutes = periodView.toInt();
            } else if (c == 'S') {
                seconds = periodView.toInt();
            }
            digitStart = -1;
        }
    }

    uint period = ((days * 24 + hours) * 60 + minutes) * 60 + seconds;
    return period;
}

QString DataUtils::formatDateTime(const QDateTime &dt) {
    const qint64 seconds = dt.secsTo(QDateTime::currentDateTimeUtc());
    QString s;
    int f = 60;
    if (seconds < f) {
        s = QCoreApplication::translate("DataUtils", "Just now");
    } else if (seconds < (f *= 60)) {
        s = QCoreApplication::translate("DataUtils", "%n minute(s) ago", Q_NULLPTR, seconds / 60);
    } else if (seconds < (f *= 24)) {
        int n = seconds / (60 * 60);
        s = QCoreApplication::translate("DataUtils", "%n hour(s) ago", Q_NULLPTR, n);
    } else if (seconds < (f *= 7)) {
        int n = seconds / (60 * 60 * 24);
        s = QCoreApplication::translate("DataUtils", "%n day(s) ago", Q_NULLPTR, n);
    } else if (seconds < (f = 60 * 60 * 24 * 30)) {
        int n = seconds / (60 * 60 * 24 * 7);
        s = QCoreApplication::translate("DataUtils", "%n week(s) ago", Q_NULLPTR, n);
    } else if (seconds < (f = 60 * 60 * 24 * 365)) {
        int n = seconds / (60 * 60 * 24 * 30);
        s = QCoreApplication::translate("DataUtils", "%n month(s) ago", Q_NULLPTR, n);
    } else {
        int n = seconds / (60 * 60 * 24 * 30 * 12);
        s = QCoreApplication::translate("DataUtils", "%n year(s) ago", Q_NULLPTR, n);
    }
    return s;
}

QString DataUtils::formatDuration(uint secs) {
    uint d = secs;
    QString res;
    uint seconds = d % 60;
    d /= 60;
    uint minutes = d % 60;
    d /= 60;
    uint hours = d % 24;
    if (hours == 0) return res.asprintf("%d:%02d", minutes, seconds);
    return res.asprintf("%d:%02d:%02d", hours, minutes, seconds);
}

QString DataUtils::formatCount(int c) {
    QString s;
    int f = 1;
    if (c < 1) {
        return s;
    } else if (c < (f *= 1000)) {
        s = QString::number(c);
    } else if (c < (f *= 1000)) {
        int n = c / 1000;
        s = QString::number(n) +
            QCoreApplication::translate("DataUtils", "K", "K as in Kilo, i.e. thousands");
    } else if (c < (f *= 1000)) {
        int n = c / (1000 * 1000);
        s = QString::number(n) +
            QCoreApplication::translate("DataUtils", "M", "M stands for Millions");
    } else {
        int n = c / (1000 * 1000 * 1000);
        s = QString::number(n) +
            QCoreApplication::translate("DataUtils", "B", "B stands for Billions");
    }

    return QCoreApplication::translate("DataUtils", "%1 views").arg(s);
}
