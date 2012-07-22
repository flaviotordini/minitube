#include "fontutils.h"

static const int MIN_PIXEL_SIZE = 11;

const QFont FontUtils::small() {
    static QFont font;
    static bool initialized = false;
    if (!initialized) {
      initialized = true;
      font.setPointSize(font.pointSize()*.85);
      if (font.pixelSize() < MIN_PIXEL_SIZE) font.setPixelSize(MIN_PIXEL_SIZE);
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
      if (font.pixelSize() < MIN_PIXEL_SIZE) font.setPixelSize(MIN_PIXEL_SIZE);
    }
    return font;
}

const QFont FontUtils::medium() {
    static QFont font;
    static bool initialized = false;
    if (!initialized) {
      initialized = true;
      font.setPointSize(font.pointSize()*1.1);
    }
    return font;
}

const QFont FontUtils::mediumBold() {
    static QFont font;
    static bool initialized = false;
    if (!initialized) {
      initialized = true;
      font.setPointSize(font.pointSize()*0.9);
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
