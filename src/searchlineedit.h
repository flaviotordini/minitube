#ifndef SEARCHLINEEDIT_H
#define SEARCHLINEEDIT_H

#include "urllineedit.h"
#include "autocomplete.h"

#include <QLineEdit>
#include <QAbstractButton>

QT_BEGIN_NAMESPACE
class QMenu;
QT_END_NAMESPACE

class SearchButton;
class Suggester;

/*
    Clear button on the right hand side of the search widget.
    Hidden by default
 */
class ClearButton : public QAbstractButton {

    Q_OBJECT

public:
    ClearButton(QWidget *parent = 0);
    void paintEvent(QPaintEvent *e);

public slots:
    void textChanged(const QString &text);

protected:
    void enterEvent(QEvent *e);
    void leaveEvent(QEvent *e);

    void mousePressEvent(QEvent *e);
    void mouseReleaseEvent(QEvent *e);

private:
    bool hovered;
    bool mousePressed;
};

class SearchLineEdit : public ExLineEdit {

    Q_OBJECT

signals:
    void textChanged(const QString &text);
    void textEdited(const QString &text);
    void search(const QString &text);
    void suggestionAccepted(Suggestion *suggestion);

public:
    SearchLineEdit(QWidget *parent = 0);

    void setInactiveText(const QString &text);

    QMenu *menu() const;
    void setMenu(QMenu *menu);
    void updateGeometries();
    void enableSuggest();
    void preventSuggest();
    void selectAll() { lineEdit()->selectAll(); }
    void setSuggester(Suggester *suggester) { autoComplete->setSuggester(suggester); }
    void setText(const QString &text) { lineEdit()->setText(text); }

protected:
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
    void focusInEvent(QFocusEvent *event);

private slots:
    void returnPressed();

private:
    SearchButton *searchButton;
    QString inactiveText;
    AutoComplete *autoComplete;
};

#endif // SEARCHLINEEDIT_H

