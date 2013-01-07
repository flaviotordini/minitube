#include "ytfeedreader.h"
#include "video.h"

YTFeedReader::YTFeedReader(const QByteArray &bytes) : QXmlStreamReader(bytes) {
    while (!atEnd()) {
        readNext();
        if (isStartElement() && name() == "entry") {
            readEntry();
        } else if (name() == "link"
                   && attributes().value("rel").toString()
                   == "http://schemas.google.com/g/2006#spellcorrection") {
            suggestions << attributes().value("title").toString();
        }
    }
}

void YTFeedReader::readEntry() {
    Video* video = new Video();

    while (!atEnd()) {
        readNext();

        /*
        qDebug() << name();
        QXmlStreamAttribute attribute;
        foreach (attribute, attributes())
            qDebug() << attribute.name() << ":" << attribute.value();
        */

        if (isEndElement() && name() == "entry") break;
        if (isStartElement()) {

            if (name() == "link"
                    && attributes().value("rel").toString() == "alternate"
                    && attributes().value("type").toString() == "text/html"
                    ) {
                QString webpage = attributes().value("href").toString();
                webpage.remove("&feature=youtube_gdata");
                video->setWebpage(QUrl(webpage));
            } else if (name() == "author") {
                while(readNextStartElement())
                    if (name() == "name") {
                        QString author = readElementText();
                        video->setAuthor(author);
                    } else if (name() == "uri") {
                        QString uri = readElementText();
                        int i = uri.lastIndexOf('/');
                        if (i != -1) uri = uri.mid(i+1);
                        video->setAuthorUri(uri);
                    } else skipCurrentElement();
            } else if (name() == "published") {
                video->setPublished(QDateTime::fromString(readElementText(), Qt::ISODate));
            } else if (namespaceUri() == "http://gdata.youtube.com/schemas/2007"
                       && name() == "statistics") {
                QString viewCount = attributes().value("viewCount").toString();
                video->setViewCount(viewCount.toInt());
            }
            else if (namespaceUri() == "http://search.yahoo.com/mrss/" && name() == "group") {

                // read media group
                while (!atEnd()) {
                    readNext();
                    if (isEndElement() && name() == "group") break;
                    if (isStartElement()) {
                        if (name() == "thumbnail") {
                            // qDebug() << "Thumb: " << attributes().value("url").toString();
                            QStringRef name = attributes().value("yt:name");
                            if (name == "default")
                                video->setThumbnailUrl(
                                            attributes().value("url").toString());
                            else if (name == "hqdefault")
                                video->setMediumThumbnailUrl(
                                            attributes().value("url").toString());
                        }
                        else if (name() == "title") {
                            QString title = readElementText();
                            // qDebug() << "Title: " << title;
                            video->setTitle(title);
                        }
                        else if (name() == "description") {
                            QString desc = readElementText();
                            // qDebug() << "Description: " << desc;
                            video->setDescription(desc);
                        }
                        else if (name() == "duration") {
                            QString duration = attributes().value("seconds").toString();
                            // qDebug() << "Duration: " << duration;
                            video->setDuration(duration.toInt());
                        }
                    }
                }
            }
        }
    }

    videos.append(video);

}

const QList<Video *> &YTFeedReader::getVideos() {
    return videos;
}

const QStringList & YTFeedReader::getSuggestions() const {
    return suggestions;
}
