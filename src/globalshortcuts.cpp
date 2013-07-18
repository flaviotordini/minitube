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

#include "globalshortcuts.h"
#include "globalshortcutbackend.h"

static GlobalShortcuts *singleton = 0;

GlobalShortcuts& GlobalShortcuts::instance() {
    if (!singleton) singleton = new GlobalShortcuts();
    return *singleton;
}

GlobalShortcuts::GlobalShortcuts(QObject *parent)
    : QObject(parent),
    backend(0) {

    // Create actions
    AddShortcut("play", tr("Play"), SIGNAL(Play()));
    AddShortcut("pause", tr("Pause"), SIGNAL(Pause()));
    AddShortcut("play_pause", tr("Play/Pause"), SIGNAL(PlayPause()), QKeySequence(Qt::Key_MediaPlay));
    AddShortcut("stop", tr("Stop"), SIGNAL(Stop()), QKeySequence(Qt::Key_MediaStop));
    AddShortcut("stop_after", tr("Stop playing after current track"), SIGNAL(StopAfter()));
    AddShortcut("next_track", tr("Next track"), SIGNAL(Next()), QKeySequence(Qt::Key_MediaNext));
    AddShortcut("prev_track", tr("Previous track"), SIGNAL(Previous()), QKeySequence(Qt::Key_MediaPrevious));
    AddShortcut("inc_volume", tr("Increase volume"), SIGNAL(IncVolume()));
    AddShortcut("dec_volume", tr("Decrease volume"), SIGNAL(DecVolume()));
    AddShortcut("mute", tr("Mute"), SIGNAL(Mute()));
    AddShortcut("seek_forward", tr("Seek forward"), SIGNAL(SeekForward()));
    AddShortcut("seek_backward", tr("Seek backward"), SIGNAL(SeekBackward()));

}

void GlobalShortcuts::AddShortcut(const QString &id, const QString &name,
                                  const char* signal,
                                  const QKeySequence &default_key) {
    Shortcut shortcut;
    shortcut.action = new QAction(name, this);
    shortcut.action->setShortcut(default_key);
    shortcut.id = id;
    shortcut.default_key = default_key;

    connect(shortcut.action, SIGNAL(triggered()), this, signal);

    shortcuts_[id] = shortcut;
}

void GlobalShortcuts::reload() {
    if (backend) {
        backend->Unregister();
        backend->Register();
    }
}
