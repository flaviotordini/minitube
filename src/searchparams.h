#ifndef SEARCHPARAMS_H
#define SEARCHPARAMS_H

#include <QObject>



class SearchParams : public QObject {

public:
    SearchParams();

    const QString keywords() const { return m_keywords; }
    void setKeywords( QString keywords ) { m_keywords = keywords; }

    const QString author() const { return m_author; }
    void setAuthor( QString author ) { m_author = author; }

    int sortBy() const { return m_sortBy; }
    void setSortBy( int sortBy ) { m_sortBy = sortBy; }

    enum SortBy {
        SortByRelevance = 1,
        SortByNewest,
        SortByViewCount
    };

private:
    QString m_keywords;
    QString m_author;
    int m_sortBy;

};

#endif // SEARCHPARAMS_H
