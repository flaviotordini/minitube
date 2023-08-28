/* $BEGIN_LICENSE

This file is part of Minitube.
Copyright 2023, Christopher Goldberg

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

// dbus-send --session --type=method_call --dest=org.mpris.MediaPlayer2.minitube /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.minitube.Previous
// qdbus org.mpris.MediaPlayer2.minitube

#include "mpriscontrol.h"
#include "mediaview.h"

MPRISControl::MPRISControl(QObject *parent) {

	QDBusConnection::sessionBus().registerService("org.mpris.MediaPlayer2.minitube");
	QDBusConnection::sessionBus().registerObject("/org/mpris/MediaPlayer2", this, QDBusConnection::ExportAllSlots);
	qWarning() << QDBusConnection::sessionBus().lastError().message();
	
	QMap<QString, QVariantMap> properties;
	
}

MPRISControl::~MPRISControl() {
	QDBusConnection::sessionBus().unregisterObject("/org/mpris/MediaPlayer2");
	QDBusConnection::sessionBus().unregisterService("org.mpris.MediaPlayer2.minitube");
}

void MPRISControl::PlayPause() {
	MediaView::instance()->pause();
}

void MPRISControl::Next() {
	MediaView::instance()->skip();
}

void MPRISControl::Previous() {
	MediaView::instance()->skipBackward();
}
