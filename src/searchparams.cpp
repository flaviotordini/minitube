#include "searchparams.h"

SearchParams::SearchParams() {
    m_transient = false;
    m_sortBy = SortByRelevance;
    m_duration = DurationAny;
    m_quality = QualityAny;
    m_time = TimeAny;
}

void SearchParams::setParam(QString name, QVariant value) {
    bool success = setProperty(name.toUtf8(), value);
    if (!success) qWarning() << "Failed to set property" << name << value;
}
