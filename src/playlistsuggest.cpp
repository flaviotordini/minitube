#include "playlistsuggest.h"
#include <QtXml>
#include "networkaccess.h"

namespace The {
NetworkAccess* http();
}

struct Playlist {
    QString id;
    QString title;
    QString summary;
    QString author;
    int videoCount;
};

PlaylistSuggest::PlaylistSuggest(QObject *parent) : Suggester() {

}

void PlaylistSuggest::suggest(QString query) {
    QUrl url("http://gdata.youtube.com/feeds/api/playlists/snippets");
    url.addQueryItem("v", "2");
    url.addQueryItem("q", query);
    QObject *reply = The::http()->get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(handleNetworkData(QByteArray)));
}

void PlaylistSuggest::handleNetworkData(QByteArray data) {
    QList<Playlist> playlists;

    QXmlStreamReader xml(data);
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "entry") {

            Playlist playlist = {};

            while (xml.readNextStartElement()) {
                if (xml.name() == "title") {
                    playlist.title = xml.readElementText();
                }
                else if (xml.name() == "summary") {
                    playlist.summary = xml.readElementText();
                }
                else if (xml.name() == "author") {
                    while (xml.readNextStartElement()) {
                        if (xml.name() == "name") {
                            playlist.author = xml.readElementText();
                            break;
                        }
                    }
                }
                else if (xml.name() == "playlistId") {
                    playlist.id = xml.readElementText();
                }
                else if (xml.name() == "countHint") {
                    playlist.videoCount = xml.readElementText().toInt();
                }
            }

            playlists << playlist;

        }
    }

    // emit ready(choices);
}

/* model */

class PlaylistSuggestModel : public QAbstractListModel {

    Q_OBJECT

public:
    PlaylistSuggestModel(QWidget *parent) : QAbstractListModel(parent) { }
    int rowCount(const QModelIndex &parent = QModelIndex()) const {
        Q_UNUSED(parent);
        return list.size();
    }
    int columnCount( const QModelIndex& parent = QModelIndex() ) const {
        Q_UNUSED(parent);
        return 1;
    }
    QVariant data(const QModelIndex &index, int role) const {
        Q_UNUSED(index);
        Q_UNUSED(role);
        return QVariant();
    }
    QList<Playlist> list;

};

/* delegate */

class PlaylistSuggestDelegate : public QStyledItemDelegate {

    Q_OBJECT

public:
    PlaylistSuggestDelegate(QObject* parent);
    QSize sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const {
        return QSize(0, 100);
    }
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
        QStyleOptionViewItemV4 opt = QStyleOptionViewItemV4(option);
        initStyleOption(&opt, index);
        opt.text = "";
        opt.widget->style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

        painter->save();
        painter->translate(option.rect.topLeft());

        QRect line(0, 0, option.rect.width(), option.rect.height());

        painter->restore();

    }

};
