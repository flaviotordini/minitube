#ifndef SEARCHWIDGET
#define SEARCHWIDGET

#include <QtWidgets>

class SearchButton;
class Suggester;
class Suggestion;
class AutoComplete;

class SearchWidget {

public:
    virtual QMenu *menu() const = 0;
    virtual void setMenu(QMenu *menu) = 0;
    virtual void enableSuggest() = 0;
    virtual void preventSuggest() = 0;
    virtual void selectAll() = 0;
    virtual void setSuggester(Suggester *suggester) = 0;
    virtual void setInactiveText(const QString &text) = 0;
    virtual void setText(const QString &text) = 0;
    virtual AutoComplete *getAutoComplete() = 0;
    virtual void emitTextChanged(const QString &text) = 0;
    virtual void returnPressed() = 0;
    virtual QString text() = 0;
    virtual QLineEdit *getLineEdit() = 0;
    virtual QWidget *toWidget() = 0;

signals:
    void textChanged(const QString &text);
    void textEdited(const QString &text);
    void search(const QString &text);
    void suggestionAccepted(Suggestion *suggestion);

};

#endif // SEARCHWIDGET
