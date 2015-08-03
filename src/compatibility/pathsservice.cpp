/* $BEGIN_LICENSE

This file is part of Minitube.
Copyright 2015, Flavio Tordini <flavio.tordini@gmail.com>

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

#include "pathsservice.h"

#include <QDesktopServices>

namespace {

#if QT_VERSION >= 0x050000
typedef QStandardPaths PathProvider;

#define getLocation writableLocation
#else
typedef QDesktopServices PathProvider;

#define getLocation storageLocation
#endif  // QT_VERSION >= 0x050000

}  // namespace

namespace Paths {
QString getMoviesLocation() {
    return PathProvider::getLocation(PathProvider::MoviesLocation);
}

QString getDesktopLocation() {
    return PathProvider::getLocation(PathProvider::DesktopLocation);
}

QString getHomeLocation() {
    return PathProvider::getLocation(PathProvider::HomeLocation);
}

QString getDataLocation() {
    return PathProvider::getLocation(PathProvider::DataLocation);
}

QString getPicturesLocation() {
    return PathProvider::getLocation(PathProvider::PicturesLocation);
}

QString getTempLocation() {
    return PathProvider::getLocation(PathProvider::TempLocation);
}

QString getCacheLocation() {
    return PathProvider::getLocation(PathProvider::CacheLocation);
}
}
