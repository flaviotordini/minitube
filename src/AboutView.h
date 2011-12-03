#ifndef ABOUTVIEW_H
#define ABOUTVIEW_H

#include <QtGui>
#include "View.h"
#include "constants.h"
#ifdef APP_MAC
#include "macutils.h"
#endif

class AboutView : public QWidget, public View {

    Q_OBJECT

public:
    AboutView(QWidget *parent);
    void appear() {
#ifdef APP_MAC
        mac::uncloseWindow(window()->winId());
#endif
    }
    void disappear() {}
    QMap<QString, QVariant> metadata() {
        QMap<QString, QVariant> metadata;
        metadata.insert("title", tr("About"));
        metadata.insert("description",
                        tr("What you always wanted to know about %1 and never dared to ask")
                        .arg(Constants::NAME));
        return metadata;
    }

protected:
    void paintEvent(QPaintEvent *);

};
#endif
