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

#include "autocomplete.h"
#include "suggester.h"
#ifdef APP_MAC
#include "searchlineedit_mac.h"
#else
#include "searchlineedit.h"
#endif

AutoComplete::AutoComplete(SearchLineEdit *parent, QLineEdit *editor):
    QObject(parent), editor(editor), suggester(0) {

    buddy = parent;
    enabled = true;

    popup = new QListWidget;
    popup->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    popup->setMouseTracking(true);
    popup->setWindowOpacity(.9);
    popup->installEventFilter(this);
    popup->setWindowFlags(Qt::Popup);
    popup->setFocusPolicy(Qt::NoFocus);
    popup->setFocusProxy(buddy);

    connect(popup, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(doneCompletion()));

    // connect(popup, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
    //    SLOT(currentItemChanged(QListWidgetItem *)));

    // mouse hover
    // connect(popup, SIGNAL(itemEntered(QListWidgetItem*)),
    //    SLOT(currentItemChanged(QListWidgetItem *)));

    timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(600);
    connect(timer, SIGNAL(timeout()), SLOT(autoSuggest()));
    connect(buddy, SIGNAL(textChanged(QString)), timer, SLOT(start()));

}

AutoComplete::~AutoComplete() {
    delete popup;
}

bool AutoComplete::eventFilter(QObject *obj, QEvent *ev) {
    if (obj != popup)
        return false;

    if (ev->type() == QEvent::FocusOut) {
        popup->hide();
        buddy->setFocus();
        return true;
    }

    if (ev->type() == QEvent::MouseButtonPress) {
        popup->hide();
        buddy->setFocus();
        buddy->setText(originalText);
        return true;
    }

    if (ev->type() == QEvent::KeyPress) {

        bool consumed = false;

        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(ev);
        int key = keyEvent->key();
        // qDebug() << keyEvent->text();
        switch (key) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
            if (popup->currentItem()) {
                doneCompletion();
                consumed = true;
            } else {
                buddy->setFocus();
                editor->event(ev);
                popup->hide();
            }
            break;

        case Qt::Key_Escape:
            buddy->setFocus();
            editor->setText(originalText);
            popup->hide();
            consumed = true;
            break;

        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_Home:
        case Qt::Key_End:
        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
            break;

        default:
            // qDebug() << keyEvent->text();
            buddy->setFocus();
            editor->event(ev);
            popup->hide();
            break;
        }

        return consumed;
    }

    return false;
}

void AutoComplete::showCompletion(const QStringList &choices) {

    if (choices.isEmpty())
        return;

    popup->setUpdatesEnabled(false);
    popup->clear();
    for (int i = 0; i < choices.count(); ++i) {
        QListWidgetItem * item;
        item = new QListWidgetItem(popup);
        item->setText(choices[i]);
    }
    popup->setCurrentItem(0);
    popup->adjustSize();
    popup->setUpdatesEnabled(true);

    int h = popup->sizeHintForRow(0) * choices.count() + 4;
    popup->resize(buddy->width(), h);

    popup->move(buddy->mapToGlobal(QPoint(0, buddy->height())));

    popup->setFocus();
    popup->show();
}

void AutoComplete::doneCompletion() {
    timer->stop();
    popup->hide();
    buddy->setFocus();
    QListWidgetItem *item = popup->currentItem();
    if (item) {
        buddy->setText(item->text());
        emit suggestionAccepted(item->text());
    }
}

void AutoComplete::preventSuggest() {
    // qDebug() << "preventSuggest";
    timer->stop();
    enabled = false;
    popup->hide();
}

void AutoComplete::enableSuggest() {
    // qDebug() << "enableSuggest";
    enabled = true;
}

void AutoComplete::setSuggester(Suggester* suggester) {
    if (this->suggester) this->suggester->disconnect();
    this->suggester = suggester;
    connect(suggester, SIGNAL(ready(QStringList)), SLOT(suggestionsReady(QStringList)));
}

void AutoComplete::autoSuggest() {
    if (!enabled) return;
    if (!buddy->hasFocus()) return;

    QString query = editor->text();
    originalText = query;
    // qDebug() << "originalText" << originalText;
    if (query.isEmpty()) {
        popup->hide();
        buddy->setFocus();
        return;
    }

    if (suggester)
        suggester->suggest(query);
}

void AutoComplete::suggestionsReady(QStringList suggestions) {
    if (!enabled) return;
    showCompletion(suggestions);
}

void AutoComplete::currentItemChanged(QListWidgetItem *current) {
    if (current) {
        // qDebug() << "current" << current->text();
        current->setSelected(true);
        buddy->setText(current->text());
        editor->setSelection(originalText.length(), editor->text().length());
    }
}
