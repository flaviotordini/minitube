#include "seekslider.h"

class MyProxyStyle : public QProxyStyle {
public:
    int styleHint(StyleHint hint,
                  const QStyleOption *option = nullptr,
                  const QWidget *widget = nullptr,
                  QStyleHintReturn *returnData = nullptr) const;
};

int MyProxyStyle::styleHint(QStyle::StyleHint hint,
                            const QStyleOption *option,
                            const QWidget *widget,
                            QStyleHintReturn *returnData) const {
    if (hint == SH_Slider_AbsoluteSetButtons) return Qt::LeftButton;
    return QProxyStyle::styleHint(hint, option, widget, returnData);
}

SeekSlider::SeekSlider(QWidget *parent) : QSlider(parent) {
    setStyle(new MyProxyStyle());
}
