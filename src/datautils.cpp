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

    if (!f.isEmpty() && f.at(0) == '.')
        f = f.mid(1).trimmed();

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
    // QTime time = QTime::fromString("1mm12car00", "PT8M50S");
    // ^P((\d+Y)?(\d+M)?(\d+W)?(\d+D)?)?(T(\d+H)?(\d+M)?(\d+S)?)?$
    /*
    QRegExp isoPeriodRE("^PT(\d+H)?(\d+M)?(\d+S)?)?$");
    if (!isoPeriodRE.indexIn(isoPeriod)) {
        qWarning() << "Cannot parse ISO period" << isoPeriod;
        continue;
    }

    int totalCaptures = isoPeriodRE.capturedTexts();
    for (int i = totalCaptures; i > 0; --i) {

    }
    */

    uint days = 0, hours = 0, minutes = 0, seconds = 0;

    QByteArray ba = isoPeriod.toLocal8Bit();
    const char *ptr = ba.data();
    
    while (*ptr) {
        if(*ptr == 'P' || *ptr == 'T') {
            ptr++;
            continue;
        }

        int value, charsRead;
        char type;
        if (sscanf(ptr, "%d%c%n", &value, &type, &charsRead) != 2)
            continue;

        if (type == 'D')
            days = value;
        else if (type == 'H')
            hours = value;
        else if (type == 'M')
            minutes = value;
        else if (type == 'S')
            seconds = value;

        ptr += charsRead;
    }

    uint period = ((days * 24 + hours) * 60 + minutes) * 60 + seconds;
    return period;
}

QString DataUtils::formatDateTime(const QDateTime &dt) {
    const qint64 seconds = dt.secsTo(QDateTime::currentDateTime());
    QString s;
    int f = 60;
    if (seconds < f) {
        s = qApp->tr("Just now");
    } else if (seconds < (f *= 60)) {
        s = qApp->tr("%n minute(s) ago", "", seconds / 60);
    } else if (seconds < (f *= 24)) {
        s = qApp->tr("%n hour(s) ago", "", seconds / (60*60));
    } else if (seconds < (f *= 7)) {
        s = qApp->tr("%n day(s) ago", "", seconds / (60*60*24));
    } else if (seconds < (f = 60*60*24*30)) {
        s = qApp->tr("%n weeks(s) ago", "", seconds / (60*60*24*7));
    } else if (seconds < (f = 60*60*24*365)) {
        s = qApp->tr("%n month(s) ago", "", seconds / (60*60*24*30));
    } else {
        s = dt.date().toString(Qt::DefaultLocaleShortDate);
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
    if (hours == 0)
        return res.sprintf("%d:%02d", minutes, seconds);
    return res.sprintf("%d:%02d:%02d", hours, minutes, seconds);
}
