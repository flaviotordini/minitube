#ifndef REFINESEARCHWIDGET_H
#define REFINESEARCHWIDGET_H

#include <QtGui>

class SearchParams;

class RefineSearchWidget : public QWidget {

    Q_OBJECT

public:
    RefineSearchWidget(QWidget *parent = 0);

    bool isDirty() { return dirty; }
    void setDirty(bool dirty) { this->dirty = dirty; }
    void setSearchParams(SearchParams* params);

signals:
    void paramChanged(QString name, QVariant value);
    void searchRefined();
    void done();
    
protected:
    void paintEvent(QPaintEvent *);

private slots:
    void actionTriggered(QAction* action);
    void doneClicked();

private:
    void setup();
    void setupLabel(QString text, QBoxLayout* layout, QString paramName);
    QToolBar *setupBar(QString paramName);

    QHash<QString, QToolBar*> bars;
    bool dirty;

};

#endif // REFINESEARCHWIDGET_H
