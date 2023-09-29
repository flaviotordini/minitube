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


#ifndef MPRISCONTROL_H
#define MPRISCONTROL_H

#include <QtDBus>

class MPRISControl : public QObject {
	Q_OBJECT

public:
	MPRISControl(QObject *parent);
	~MPRISControl();

public slots:
	void PlayPause();
	void Next();
	void Previous();

//private:

};

#endif // MPRISCONTROL_H
