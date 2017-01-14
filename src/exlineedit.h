#ifndef EXLINEEDIT_H
#define EXLINEEDIT_H

#include <QtWidgets>

class ClearButton : public QAbstractButton {

    Q_OBJECT

public:
    ClearButton(QWidget *parent = 0);

public slots:
    void textChanged(const QString &text);

protected:
    void paintEvent(QPaintEvent *e);
    void enterEvent(QEvent *e);
    void leaveEvent(QEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);

private:
    bool hovered;
    bool mousePressed;
};

class ExLineEdit : public QWidget {

    Q_OBJECT

public:
    ExLineEdit(QWidget *parent = 0);
    QLineEdit *lineEdit() const { return m_lineEdit; }
    void setLeftWidget(QWidget *widget);
    QWidget *leftWidget() const;
    void clear();
    QString text();
    QSize sizeHint() const;
    void updateGeometries();
    void setFont(const QFont &font);

protected:
    void focusInEvent(QFocusEvent *e);
    void focusOutEvent(QFocusEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *e);
    bool event(QEvent *e);
    void initStyleOption(QStyleOptionFrameV2 *option) const;

    QWidget *m_leftWidget;
    QLineEdit *m_lineEdit;
    ClearButton *m_clearButton;
};

#endif // EXLINEEDIT_H

