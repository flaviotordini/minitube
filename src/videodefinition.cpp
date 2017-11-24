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

#include "videodefinition.h"

namespace {
static const int kEmptyDefinitionCode = -1;

static const VideoDefinition kEmptyDefinition(QString(), kEmptyDefinitionCode);

template <typename T, T (VideoDefinition::*Getter)() const>
const VideoDefinition &getDefinitionForImpl(T matchValue) {
    const auto &defs = VideoDefinition::getDefinitions();
    for (const VideoDefinition &def : defs) {
        if ((def.*Getter)() == matchValue) return def;
    }
    return kEmptyDefinition;
}
}

// static
const QVector<VideoDefinition> &VideoDefinition::getDefinitions() {
    static const QVector<VideoDefinition> definitions = {
            VideoDefinition(QLatin1String("360p"), 18), VideoDefinition(QLatin1String("720p"), 22),
            VideoDefinition(QLatin1String("1080p"), 37)};
    return definitions;
}

// static
const VideoDefinition &VideoDefinition::forName(const QString &name) {
    return getDefinitionForImpl<const QString &, &VideoDefinition::getName>(name);
}

// static
const VideoDefinition &VideoDefinition::forCode(int code) {
    return getDefinitionForImpl<int, &VideoDefinition::getCode>(code);
}

VideoDefinition::VideoDefinition(const QString &name, int code) : m_name(name), m_code(code) {}

bool VideoDefinition::isEmpty() const {
    return m_code == kEmptyDefinitionCode && m_name.isEmpty();
}
