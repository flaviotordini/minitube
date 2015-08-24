#ifndef SEARCHLINEEDIT_H
#define SEARCHLINEEDIT_H

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

#include "exlineedit.h"
#include "searchwidget.h"

class SearchButton;
class Suggester;
class AutoComplete;

class SearchLineEdit : public ExLineEdit, public SearchWidget {

    Q_OBJECT

public:
    SearchLineEdit(QWidget *parent = 0);
    QMenu *menu() const;
    void setMenu(QMenu *menu);
    void enableSuggest();
    void preventSuggest();
    void selectAll();
    void setSuggester(Suggester *suggester);
    void setInactiveText(const QString &text);
    void setText(const QString &text);
    AutoComplete *getAutoComplete();
    void emitTextChanged(const QString &text);
    QString text();
    QLineEdit *getLineEdit();

public slots:
    void returnPressed();

signals:
    void textChanged(const QString &text);
    void textEdited(const QString &text);
    void search(const QString &text);
    void suggestionAccepted(Suggestion *suggestion);

protected:
    void updateGeometries();
    void resizeEvent(QResizeEvent *e);
    void paintEvent(QPaintEvent *e);
    void focusInEvent(QFocusEvent *e);

private:
    SearchButton *searchButton;
    QString inactiveText;
    AutoComplete *autoComplete;
};

#endif // SEARCHLINEEDIT_H

