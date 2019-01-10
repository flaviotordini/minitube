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
const int kEmptyDefinitionCode = -1;

template <typename T, T (VideoDefinition::*Getter)() const>
const VideoDefinition &getDefinitionForImpl(T matchValue) {
    const auto &defs = VideoDefinition::getDefinitions();

    for (auto i = defs.rbegin(); i != defs.rend(); ++i) {
        if ((*i.*Getter)() == matchValue) return *i;
    }
    /*
    for (const VideoDefinition &def : defs) {
        if ((def.*Getter)() == matchValue) return def;
    }
    */
    static const VideoDefinition kEmptyDefinition(QString(), kEmptyDefinitionCode);
    return kEmptyDefinition;
}
} // namespace

// static
const QVector<VideoDefinition> &VideoDefinition::getDefinitions() {
    static const QVector<VideoDefinition> definitions = {
            VideoDefinition(QLatin1String("240p"), 133, 140),
            VideoDefinition(QLatin1String("360p"), 396, 140),
            VideoDefinition(QLatin1String("360p"), 18),
            VideoDefinition(QLatin1String("480p"), 135, 140),
            VideoDefinition(QLatin1String("720p"), 136, 140),
            VideoDefinition(QLatin1String("720p"), 22),
            VideoDefinition(QLatin1String("1080p"), 137, 140)};
    return definitions;
}

const QVector<QString> &VideoDefinition::getDefinitionNames() {
    static const QVector<QString> names = {"240p", "360p", "480p", "720p", "1080p"};
    return names;
}

// static
const VideoDefinition &VideoDefinition::forName(const QString &name) {
    return getDefinitionForImpl<const QString &, &VideoDefinition::getName>(name);
}

// static
const VideoDefinition &VideoDefinition::forCode(int code) {
    return getDefinitionForImpl<int, &VideoDefinition::getCode>(code);
}

VideoDefinition::VideoDefinition(const QString &name, int code, int audioCode)
    : m_name(name), m_code(code), audioCode(audioCode) {}

bool VideoDefinition::isEmpty() const {
    return m_code == kEmptyDefinitionCode && m_name.isEmpty();
}

const VideoDefinition VideoDefinition::preferred() {
    const QString definitionName = QSettings().value("definition", "720p").toString();
    const VideoDefinition &definition = VideoDefinition::forName(definitionName);
    return definition;
}

void VideoDefinition::savePreferred(const QString &definitionName) {
    QSettings settings;
    settings.setValue("definition", definitionName);
}
