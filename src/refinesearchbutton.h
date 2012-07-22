#ifndef REFINESEARCHBUTTON_H
#define REFINESEARCHBUTTON_H

#include <QtGui>

class RefineSearchButton : public QPushButton
{
    Q_OBJECT
public:
    RefineSearchButton(QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *);
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    
private:
    void paintBackground() const;
    bool hovered;

};

#endif // REFINESEARCHBUTTON_H
