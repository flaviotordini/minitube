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

#ifndef VIDEODEFINITION_H
#define VIDEODEFINITION_H

#include <QtCore>

class VideoDefinition {
public:
    static const QVector<VideoDefinition> &getDefinitions();
    static const VideoDefinition &forName(const QString &name);
    static const VideoDefinition &forCode(int code);

    VideoDefinition(const QString &name, int code);

    const QString &getName() const { return m_name; }
    int getCode() const { return m_code; }
    bool isEmpty() const;

    VideoDefinition &operator=(const VideoDefinition &);

private:
    const QString m_name;
    const int m_code;
};

inline bool operator==(const VideoDefinition &lhs, const VideoDefinition &rhs) {
    return lhs.getCode() == rhs.getCode();
}

#endif // VIDEODEFINITION_H
