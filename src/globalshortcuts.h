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

#ifndef GLOBALSHORTCUTS_H
#define GLOBALSHORTCUTS_H

#include <QtCore>
#include <QAction>

class GlobalShortcutBackend;

class GlobalShortcuts : public QObject {

    Q_OBJECT

public:
    static GlobalShortcuts& instance();

    struct Shortcut {
        QString id;
        QKeySequence default_key;
        QAction* action;
    };

    const QMap<QString, Shortcut> &shortcuts() const { return shortcuts_; }
    void setBackend(GlobalShortcutBackend* backend) {
        this->backend = backend;
        reload();
    }

public slots:
    void reload();

signals:
    void Play();
    void Pause();
    void PlayPause();
    void Stop();
    void StopAfter();
    void Next();
    void Previous();
    void IncVolume();
    void DecVolume();
    void Mute();
    void SeekForward();
    void SeekBackward();

private:
    GlobalShortcuts(QObject* parent = 0);
    void AddShortcut(const QString& id, const QString& name, const char* signal,
                     const QKeySequence& default_key = QKeySequence(0));

private:
    GlobalShortcutBackend* backend;

    QMap<QString, Shortcut> shortcuts_;
};

#endif
