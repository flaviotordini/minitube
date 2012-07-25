#ifndef VIDEODEFINITION_H
#define VIDEODEFINITION_H

#include <QtCore>

class VideoDefinition {

public:
    static QStringList getDefinitionNames();
    static QList<int> getDefinitionCodes();
    static QHash<QString, int> getDefinitions();
    static int getDefinitionCode(QString name);
    static QString getDefinitionName(int code);

};

#endif // VIDEODEFINITION_H
