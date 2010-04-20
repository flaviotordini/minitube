#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>

namespace Constants {
    static const char *VERSION = "1.0";
    static const char *APP_NAME = "Minitube";
    static const char *ORG_NAME = "Flavio Tordini";
    static const char *ORG_DOMAIN = "flavio.tordini.org";
    static const char *WEBSITE = "http://flavio.tordini.org/minitube";
    static const char *EMAIL = "flavio.tordini@gmail.com";
    static const QString USER_AGENT = QString(APP_NAME) + " " + VERSION + " (" + WEBSITE + ")";
}

#endif
