#ifndef VIEW_H
#define VIEW_H

class View {

    public:
        virtual QMap<QString, QVariant> metadata() = 0;
        virtual void appear() = 0;
        virtual void disappear() = 0;

};

#endif // VIEW_H
