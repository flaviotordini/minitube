#ifndef SEARCHWIDGET
#define SEARCHWIDGET

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

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

    QWidget *toWidget() {
        return dynamic_cast<QWidget*>(this);
    }

signals:
    void textChanged(const QString &text);
    void textEdited(const QString &text);
    void search(const QString &text);
    void suggestionAccepted(Suggestion *suggestion);

};
/*
class SearchWidget : public QWidget, public SearchWidgetInterface {

public:
    SearchWidget(QWidget *parent = 0);
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
    void returnPressed();
    QString text();

private:
    SearchWidgetInterface *interface;

};
*/

#endif // SEARCHWIDGET
