#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QtWidgets>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
typedef QEnterEvent CompatibleEnterEvent;
#else
typedef QEvent CompatibleEnterEvent;
#endif

class ClickableLabel : public QLabel {
    Q_OBJECT

public:
    explicit ClickableLabel(QWidget *parent = nullptr);
    explicit ClickableLabel(const QString &text, QWidget *parent = nullptr);

signals:
    void clicked();
    void hovered(bool value);

protected:
    void mouseReleaseEvent(QMouseEvent *e);
    void enterEvent(CompatibleEnterEvent *e);
    void leaveEvent(QEvent *e);
};

#endif // CLICKABLELABEL_H
