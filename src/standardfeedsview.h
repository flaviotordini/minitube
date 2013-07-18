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

#ifndef CATEGORIESVIEW_H
#define CATEGORIESVIEW_H

#include <QtGui>
#include "view.h"

class VideoSource;
class YTCategory;
class YTStandardFeed;

class StandardFeedsView : public QWidget, public View {

    Q_OBJECT

public:
    StandardFeedsView(QWidget *parent = 0);

signals:
    void activated(VideoSource *standardFeed);

public slots:
    void appear();
    void disappear();
    void load();

protected:
    void paintEvent(QPaintEvent *event);
    
private slots:
    void layoutCategories(const QList<YTCategory> &categories);
    void selectWorldwideRegion();
    void selectLocalRegion();

private:
    void addVideoSourceWidget(VideoSource *videoSource);
    QList<YTStandardFeed*> getMainFeeds();
    YTStandardFeed* buildStardardFeed(QString feedId, QString label, QString time = QString());
    QGridLayout *layout;
    
};

#endif // CATEGORIESVIEW_H
