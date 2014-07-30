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

#ifndef SIDEBARWIDGET_H
#define SIDEBARWIDGET_H

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

class RefineSearchButton;
class RefineSearchWidget;
class SidebarHeader;

class SidebarWidget : public QWidget {

    Q_OBJECT

public:
    SidebarWidget(QWidget *parent = 0);
    void setPlaylist(QListView *playlist);
    void showPlaylist();
    RefineSearchWidget* getRefineSearchWidget() { return refineSearchWidget; }
    SidebarHeader* getHeader() { return sidebarHeader; }
    void hideSuggestions();

public slots:
    void showRefineSearchWidget();
    void hideRefineSearchWidget();
    void toggleRefineSearch(bool show = false);
    void showSuggestions(const QStringList &suggestions);

signals:
    void suggestionAccepted(QString);

protected:
    void resizeEvent(QResizeEvent *);
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    void mouseMoveEvent(QMouseEvent *event);
    bool eventFilter(QObject *, QEvent *);

private:
    void showRefineSearchButton();
    void setup();
    void handleMouseMove();

    QStackedWidget *stackedWidget;
    RefineSearchButton *refineSearchButton;
    QListView *playlist;
    RefineSearchWidget *refineSearchWidget;
    QTimer *mouseTimer;
    QLabel *messageLabel;
    SidebarHeader *sidebarHeader;
};

#endif // SIDEBARWIDGET_H
