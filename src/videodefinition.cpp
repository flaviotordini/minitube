#include "videodefinition.h"

QStringList VideoDefinition::getDefinitionNames() {
    static QStringList definitionNames = QStringList() << "360p" << "720p" << "1080p";
    return definitionNames;
}

QList<int> VideoDefinition::getDefinitionCodes() {
    static QList<int> definitionCodes = QList<int>() << 34 << 22 << 37;
    return definitionCodes;
}

QHash<QString, int> VideoDefinition::getDefinitions() {
    static QHash<QString, int> definitions;
    if (definitions.isEmpty()) {
        definitions.insert("360p", 34);
        definitions.insert("720p", 22);
        definitions.insert("1080p", 37);
    }
    return definitions;
}

int VideoDefinition::getDefinitionCode(QString name) {
    return VideoDefinition::getDefinitions().value(name);
}
