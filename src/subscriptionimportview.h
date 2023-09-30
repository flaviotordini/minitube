#ifndef SUBSCRIPTIONIMPORTVIEW_H
#define SUBSCRIPTIONIMPORTVIEW_H

#include <QtWidgets>

class SubscriptionImportView : public QWidget {
    Q_OBJECT

public:
    static QAction *buildAction(QWidget *parent);
    explicit SubscriptionImportView(QWidget *parent = nullptr);

protected:
    void showEvent(QShowEvent *event);
};

#endif // SUBSCRIPTIONIMPORTVIEW_H
