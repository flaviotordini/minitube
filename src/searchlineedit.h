#ifndef SEARCHLINEEDIT_H
#define SEARCHLINEEDIT_H

#include <QtWidgets>

#include "searchwidget.h"

class SearchLineEdit : public QLineEdit, public SearchWidget {
    Q_OBJECT

public:
    explicit SearchLineEdit(QWidget *parent = nullptr);

    // SearchWidget interface
    QMenu *menu() const;
    void setMenu(QMenu *menu);
    void enableSuggest();
    void preventSuggest();
    void setSuggester(Suggester *suggester);
    AutoComplete *getAutoComplete();
    void emitTextChanged(const QString &text);
    QLineEdit *getLineEdit();
    QWidget *toWidget();

    void setPlaceholderText(const QString &text) { QLineEdit::setPlaceholderText(text); }
    void selectAll() { QLineEdit::selectAll(); }
    void setText(const QString &text) { QLineEdit::setText(text); }
    QString text() { return QLineEdit::text(); }

public slots:
    void returnPressed();

signals:
    void search(const QString &text);
    void suggestionAccepted(Suggestion *suggestion);

private:
    AutoComplete *autoComplete;
};

#endif // SEARCHLINEEDIT_H
