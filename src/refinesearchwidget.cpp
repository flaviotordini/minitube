/* $BEGIN_LICENSE

This file is part of Minitube.
Copyright 2009, Flavio Tordini <flavio.tordini@gmail.com>

Minitube is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Minitube is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Minitube.  If not, see <http://www.gnu.org/licenses/>.

$END_LICENSE */

#include "refinesearchwidget.h"
#include "fontutils.h"
#include "searchparams.h"
#ifdef APP_EXTRA
#include "extra.h"
#endif

namespace The {
QHash<QString, QAction*>* globalActions();
}

RefineSearchWidget::RefineSearchWidget(QWidget *parent) :
    QWidget(parent) {
    dirty = false;
}

void RefineSearchWidget::setup() {
    static bool isSetup = false;
    if (isSetup) return;
    isSetup = true;

    static const int spacing = 15;
    setFont(FontUtils::medium());

    QBoxLayout *layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    layout->setMargin(spacing*2);
    layout->setSpacing(spacing);

    QString paramName = "sortBy";
    setupLabel(tr("Sort by"), layout, paramName);
    QToolBar *sortBar = setupBar(paramName);
    QActionGroup* sortGroup = new QActionGroup(this);
    QStringList sortOptions = QStringList()
            << tr("Relevance")
            << tr("Date")
            << tr("View Count")
            << tr("Rating");
    int i = 0;
    foreach (QString actionName, sortOptions) {
        QAction *action = new QAction(actionName, sortBar);
        action->setCheckable(true);
        action->setFont(FontUtils::medium());
        action->setProperty("paramValue", i);
        sortGroup->addAction(action);
        sortBar->addAction(action);
        i++;
    }

    paramName = "time";
    layout->addSpacing(spacing);
    setupLabel(tr("Date"), layout, paramName);
    QToolBar *timeBar = setupBar(paramName);
    QActionGroup* timeGroup = new QActionGroup(this);
    QStringList timeSpans = QStringList()
            << tr("Anytime")
            << tr("Today")
            << tr("7 Days")
            << tr("30 Days");
    i = 0;
    foreach (QString actionName, timeSpans) {
        QAction *action = new QAction(actionName, timeBar);
        action->setCheckable(true);
        action->setFont(FontUtils::medium());
        action->setProperty("paramValue", i);
        timeGroup->addAction(action);
        timeBar->addAction(action);
        i++;
    }

    paramName = "duration";
    layout->addSpacing(spacing);
    setupLabel(tr("Duration"), layout, paramName);
    QToolBar *lengthBar = setupBar(paramName);
    QActionGroup* lengthGroup = new QActionGroup(this);
    QStringList lengthOptions = QStringList()
            << tr("All")
            << tr("Short")
            << tr("Medium")
            << tr("Long");
    QStringList tips = QStringList()
            << ""
            << tr("Less than 4 minutes")
            << tr("Between 4 and 20 minutes")
            << tr("Longer than 20 minutes");
    i = 0;
    foreach (QString actionName, lengthOptions) {
        QAction *action = new QAction(actionName, timeBar);
        action->setStatusTip(tips.at(i));
        action->setCheckable(true);
        action->setFont(FontUtils::medium());
        action->setProperty("paramValue", i);
        lengthGroup->addAction(action);
        lengthBar->addAction(action);
        i++;
    }

    paramName = "quality";
    layout->addSpacing(spacing);
    setupLabel(tr("Quality"), layout, paramName);
    QToolBar *qualityBar = setupBar(paramName);
    QActionGroup* qualityGroup = new QActionGroup(this);
    QStringList qualityOptions = QStringList()
            << tr("All")
            << tr("High Definition");
    tips = QStringList()
            << ""
            << tr("720p or higher");
    i = 0;
    foreach (QString actionName, qualityOptions) {
        QAction *action = new QAction(actionName, timeBar);
        action->setStatusTip(tips.at(i));
        action->setCheckable(true);
        action->setFont(FontUtils::medium());
        action->setProperty("paramValue", i);
        qualityGroup->addAction(action);
        qualityBar->addAction(action);
        i++;
    }

    layout->addSpacing(spacing);
    doneButton = new QPushButton(tr("Done"), this);
    doneButton->setDefault(true);
    doneButton->setAutoDefault(true);
    doneButton->setFocusPolicy(Qt::StrongFocus);
    doneButton->setFont(FontUtils::medium());
    doneButton->setProperty("custom", true);
    doneButton->setProperty("important", true);
    doneButton->setProperty("big", true);
    connect(doneButton, SIGNAL(clicked()), SLOT(doneClicked()));
    layout->addWidget(doneButton, 0, Qt::AlignLeft);
}

void RefineSearchWidget::setupLabel(QString text, QBoxLayout *layout, QString paramName) {
    QBoxLayout* hLayout = new QHBoxLayout();
    hLayout->setSpacing(8);
    hLayout->setMargin(0);
    hLayout->setAlignment(Qt::AlignVCenter);

    QLabel *icon = new QLabel(this);
    icon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QString resource = paramName;
#ifdef APP_EXTRA
    resource = Extra::resourceName(resource);
#endif
    QPixmap pixmap = QPixmap(":/images/search-" + resource + ".png");
    QPixmap translucentPixmap(pixmap.size());
    translucentPixmap.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&translucentPixmap);
    painter.setOpacity(0.5);
    painter.drawPixmap(0, 0, pixmap);
    painter.end();
    icon->setPixmap(translucentPixmap);
    hLayout->addWidget(icon);

    QLabel *label = new QLabel(text.toUpper(), this);
    label->setFont(FontUtils::mediumBold());
    label->setStyleSheet("color: rgba(0, 0, 0, 128);");
    hLayout->addWidget(label);

    icon->setMaximumHeight(label->height());

    layout->addLayout(hLayout);
}

QToolBar* RefineSearchWidget::setupBar(QString paramName) {
    QToolBar* bar = new QToolBar(this);
    bar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    // bar->setProperty("segmented", true);
    bar->setFont(FontUtils::medium());
    bar->setProperty("paramName", paramName);
    connect(bar, SIGNAL(actionTriggered(QAction*)),
            SLOT(actionTriggered(QAction*)));
    bars.insert(paramName, bar);
    layout()->addWidget(bar);
    return bar;
}

void RefineSearchWidget::paintEvent(QPaintEvent * /*event*/) {
#if defined(APP_MAC) | defined(APP_WIN)
    QBrush brush;
    if (window()->isActiveWindow()) {
        brush = QBrush(QColor(0xdd, 0xe4, 0xeb));
    } else {
        brush = palette().window();
    }
    QPainter painter(this);
    painter.fillRect(0, 0, width(), height(), brush);
#endif
}

void RefineSearchWidget::actionTriggered(QAction *action) {
    QToolBar *bar = static_cast<QToolBar *>(sender());
    if (!bar) {
        qDebug() << __PRETTY_FUNCTION__ << "Cannot get sender";
        return;
    }

    QString paramName = bar->property("paramName").toString();
    QVariant paramValue = action->property("paramValue");

    // qDebug() << "param changed" << paramName << paramValue;
    emit paramChanged(paramName, paramValue);

    dirty = true;
}

void RefineSearchWidget::setSearchParams(SearchParams *params) {
    setup();

    The::globalActions()->value("refine-search")->setEnabled(params);
    setEnabled(params);

    if (!params) return;

    QToolBar* bar;
    QAction* action;

    bar = bars.value("sortBy");
    action = bar->actions().at(params->sortBy());
    if (action) action->setChecked(true);

    bar = bars.value("duration");
    action = bar->actions().at(params->duration());
    if (action) action->setChecked(true);

    bar = bars.value("time");
    action = bar->actions().at(params->time());
    if (action) action->setChecked(true);

    bar = bars.value("quality");
    action = bar->actions().at(params->quality());
    if (action) action->setChecked(true);

    disconnect(SIGNAL(paramChanged(QString,QVariant)));
    connect(this, SIGNAL(paramChanged(QString,QVariant)),
            params, SLOT(setParam(QString,QVariant)),
            Qt::UniqueConnection);

    dirty = false;

    doneButton->setFocus();
}

void RefineSearchWidget::doneClicked() {
    if (dirty) emit searchRefined();
    emit done();
}
