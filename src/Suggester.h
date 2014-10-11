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

#ifndef SUGGESTER_H
#define SUGGESTER_H

#include <QtCore>

class Suggestion {

public:
    Suggestion(QString value = QString(),
               QString type = QString()) :
        value(value), type(type) { }
    QString value;
    QString type;

    bool operator==(const Suggestion &other) const {
        return (value == other.value) && (type == other.type);
    }

    bool operator!=(const Suggestion &other) const {
        return !(*this == other);
    }

    bool operator==(Suggestion *other) const {
        qDebug() << "Comparing" << this << other;
        return (value == other->value) && (type == other->type);
    }

    bool operator!=(Suggestion *other) const {
        return !(this == other);
    }

};

class Suggester : public QObject {

    Q_OBJECT

public:
    Suggester(QObject *parent) : QObject(parent) { }
    virtual void suggest(const QString &query) = 0;

signals:
    void ready(const QList<Suggestion*> &suggestions);

};

#endif // SUGGESTER_H

