#ifndef VIEW_H
#define VIEW_H

class View {

public:
    virtual QHash<QString, QVariant> metadata() { return QHash<QString, QVariant>(); }
    virtual void appear() {}
    virtual void disappear() {}

};

#endif // VIEW_H
