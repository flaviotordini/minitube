#include "fontutils.h"

const QFont FontUtils::small() {
    static QFont font;
    static bool initialized = false;
    if (!initialized) {
      initialized = true;
      font.setPointSize(font.pointSize()*.85);
    }
    return font;
}

const QFont FontUtils::smallBold() {
    static QFont font;
    static bool initialized = false;
    if (!initialized) {
      initialized = true;
      font.setPointSize(font.pointSize()*.85);
      font.setBold(true);
    }
    return font;
}

const QFont FontUtils::big() {
    static QFont font;
    static bool initialized = false;
    if (!initialized) {
      initialized = true;
      font.setPointSize(font.pointSize()*1.5);
    }
    return font;
}

const QFont FontUtils::bigBold() {
    static QFont font;
    static bool initialized = false;
    if (!initialized) {
      initialized = true;
      font.setPointSize(font.pointSize()*1.5);
      font.setBold(true);
    }
    return font;
}
