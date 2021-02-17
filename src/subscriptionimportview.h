#ifndef SUBSCRIPTIONIMPORTVIEW_H
#define SUBSCRIPTIONIMPORTVIEW_H

#include <QtWidgets>

#include "view.h"

class SubscriptionImportView : public View {
    Q_OBJECT

public:
    static QAction *buildAction(QWidget *parent);
    explicit SubscriptionImportView(QWidget *parent = nullptr);

};

#endif // SUBSCRIPTIONIMPORTVIEW_H
