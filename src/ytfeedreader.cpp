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

#include "ytfeedreader.h"
#include "video.h"

YTFeedReader::YTFeedReader(const QByteArray &bytes) : QXmlStreamReader(bytes) {
    while (!atEnd()) {
        readNext();
        if (isStartElement() && name() == QLatin1String("entry")) {
            readEntry();
        } else if (name() == QLatin1String("link")
                   && attributes().value("rel").toString()
                   == QLatin1String("http://schemas.google.com/g/2006#spellcorrection")) {
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

        if (isEndElement() && name() == QLatin1String("entry")) break;
        if (isStartElement()) {

            if (name() == QLatin1String("link")
                    && attributes().value("rel").toString() == QLatin1String("alternate")
                    && attributes().value("type").toString() == QLatin1String("text/html")
                    ) {
                QString webpage = attributes().value("href").toString();
                webpage.remove("&feature=youtube_gdata");
                video->setWebpage(QUrl(webpage));
            } else if (name() == QLatin1String("author")) {
                while(readNextStartElement())
                    if (name() == QLatin1String("name")) {
                        QString author = readElementText();
                        video->setAuthor(author);
                    } else if (name() == QLatin1String("userId")) {
                        QString userId = readElementText();
                        video->setUserId(userId);
                    } else skipCurrentElement();
            } else if (name() == QLatin1String("published")) {
                video->setPublished(QDateTime::fromString(readElementText(), Qt::ISODate));
            } else if (namespaceUri() == QLatin1String("http://gdata.youtube.com/schemas/2007")
                       && name() == QLatin1String("statistics")) {
                QString viewCount = attributes().value("viewCount").toString();
                video->setViewCount(viewCount.toInt());
            }
            else if (namespaceUri() == QLatin1String("http://search.yahoo.com/mrss/")
                     && name() == QLatin1String("group")) {

                // read media group
                while (!atEnd()) {
                    readNext();
                    if (isEndElement() && name() == QLatin1String("group")) break;
                    if (isStartElement()) {
                        if (name() == QLatin1String("thumbnail")) {
                            // qDebug() << "Thumb: " << attributes().value("url").toString();
                            QStringRef name = attributes().value("yt:name");
                            if (name == QLatin1String("mqdefault"))
                                video->setThumbnailUrl(
                                            attributes().value("url").toString());
                            else if (name == QLatin1String("hqdefault"))
                                video->setMediumThumbnailUrl(
                                            attributes().value("url").toString());
                        }
                        else if (name() == QLatin1String("title")) {
                            QString title = readElementText();
                            // qDebug() << "Title: " << title;
                            video->setTitle(title);
                        }
                        else if (name() == QLatin1String("description")) {
                            QString desc = readElementText();
                            // qDebug() << "Description: " << desc;
                            video->setDescription(desc);
                        }
                        else if (name() == QLatin1String("duration")) {
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
