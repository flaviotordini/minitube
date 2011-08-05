#ifndef ABOUTVIEW_H
#define ABOUTVIEW_H

#include <QtGui>
#include "View.h"
#include "constants.h"

class AboutView : public QWidget, public View {

    Q_OBJECT

public:
    AboutView(QWidget *parent);
    void appear() {}
    void disappear() {}
    QMap<QString, QVariant> metadata() {
        QMap<QString, QVariant> metadata;
        metadata.insert("title", tr("About"));
        metadata.insert("description",
                        tr("What you always wanted to know about %1 and never dared to ask")
                        .arg(Constants::APP_NAME));
        return metadata;
    }

protected:
    void paintEvent(QPaintEvent *);

};
#endif
