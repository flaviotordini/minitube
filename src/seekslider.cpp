#include "seekslider.h"

class MyProxyStyle : public QProxyStyle {
public:
    int styleHint(StyleHint hint, const QStyleOption *option = 0,
                  const QWidget *widget = 0, QStyleHintReturn *returnData = 0) const {
        if (hint == SH_Slider_AbsoluteSetButtons)
                return Qt::LeftButton;
        return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
};

SeekSlider::SeekSlider(QWidget *parent) : QSlider(parent) {
    setStyle(new MyProxyStyle());
}
