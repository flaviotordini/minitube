#include "youtubestreamreader.h"
#include <QtGui>


YouTubeStreamReader::YouTubeStreamReader() {

}

bool YouTubeStreamReader::read(QByteArray data) {
    addData(data);

    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (name() == "feed") {
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
        }
    }

    return !error();
}

void YouTubeStreamReader::readMediaGroup() {

}

void YouTubeStreamReader::readEntry() {
    Video* video = new Video();
    // qDebug(" *** ENTRY ***");

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
                // qDebug() << "Webpage: " << webpage;
                video->setWebpage(QUrl(webpage));

            } else if (name() == "author") {
                readNext();
                if (name() == "name") {
                    QString author = readElementText();
                    // qDebug() << "Author: " << author;
                    video->setAuthor(author);
                }
            } else if (name() == "published") {
                video->setPublished(QDateTime::fromString(readElementText(), Qt::ISODate));
            } else if (namespaceUri() == "http://gdata.youtube.com/schemas/2007" && name() == "statistics") {

                QString viewCount = attributes().value("viewCount").toString();
                // qDebug() << "viewCount: " << viewCount;
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
                            // video->thumbnailUrls() << QUrl(attributes().value("url").toString());
                            video->addThumbnailUrl(QUrl(attributes().value("url").toString()));
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

QList<Video*> YouTubeStreamReader::getVideos() {
    return videos;
}

const QStringList & YouTubeStreamReader::getSuggestions() const {
    return suggestions;
}
