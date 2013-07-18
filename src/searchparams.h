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

#ifndef SEARCHPARAMS_H
#define SEARCHPARAMS_H

#include <QtCore>

class SearchParams : public QObject {

    Q_OBJECT
    Q_PROPERTY(int sortBy READ sortBy WRITE setSortBy)
    Q_PROPERTY(int duration READ duration WRITE setDuration)
    Q_PROPERTY(int quality READ quality WRITE setQuality)
    Q_PROPERTY(int time READ time WRITE setTime)

public:

    enum SortBy {
        SortByRelevance = 0,
        SortByNewest,
        SortByViewCount,
        SortByRating
    };

    enum Duration {
        DurationAny = 0,
        DurationShort,
        DurationMedium,
        DurationLong
    };

    enum Quality {
        QualityAny = 0,
        QualityHD
    };

    enum Time {
        TimeAny = 0,
        TimeToday,
        TimeWeek,
        TimeMonth
    };

    SearchParams(QObject *parent = 0);

    const QString keywords() const { return m_keywords; }
    void setKeywords( QString keywords ) { m_keywords = keywords; }

    const QString author() const { return m_author; }
    void setAuthor( QString author ) { m_author = author; }

    int sortBy() const { return m_sortBy; }
    void setSortBy( int sortBy ) { m_sortBy = sortBy; }

    int isTransient() const { return m_transient; }
    void setTransient( int transient ) { m_transient = transient; }

    int duration() const { return m_duration; }
    void setDuration( int duration ) { m_duration = duration; }

    int quality() const { return m_quality; }
    void setQuality( int quality ) { m_quality = quality; }

    int time() const { return m_time; }
    void setTime( int time ) { m_time = time; }

    bool operator==(const SearchParams &other) const {
        return m_keywords == other.keywords() &&
                m_author == other.author();
    }

public slots:
    void setParam(QString name, QVariant value);

private:
    QString m_keywords;
    QString m_author;
    bool m_transient;
    int m_sortBy;
    int m_duration;
    int m_quality;
    int m_time;

};

#endif // SEARCHPARAMS_H
