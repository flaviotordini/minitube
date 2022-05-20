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

#include "loadingwidget.h"
#include "fontutils.h"

LoadingWidget::LoadingWidget(QWidget *parent) : QWidget(parent) {
    QPalette p = palette();
    p.setColor(QPalette::Window, Qt::black);
    p.setColor(QPalette::WindowText, Qt::white);
    p.setColor(QPalette::Base, Qt::black);
    p.setColor(QPalette::Text, Qt::white);
    setPalette(p);
    setAutoFillBackground(true);

    QBoxLayout *layout = new QVBoxLayout(this);

    titleLabel = new QLabel();
    titleLabel->setPalette(p);
    titleLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    titleLabel->setWordWrap(true);
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    titleLabel->setTextFormat(Qt::RichText);
    titleLabel->setFont(FontUtils::light(titleLabel->font().pointSize()));
    layout->addWidget(titleLabel);

    descriptionLabel = new QLabel();
    descriptionLabel->setPalette(p);
    descriptionLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
    descriptionLabel->setTextFormat(Qt::RichText);
    descriptionLabel->setOpenExternalLinks(true);
    layout->addWidget(descriptionLabel);

    progressBar = new QProgressBar();
    progressBar->setStyleSheet("QProgressBar {max-height:3px; background:black; border:0} "
                               "QProgressBar::chunk {background:white}");
    progressBar->setTextVisible(false);
    layout->addWidget(progressBar);
}

void LoadingWidget::setVideo(Video *video) {
    adjustFontSize();

    QString title = video->getTitle();
    // enhance legibility by splitting the title
    static const QLatin1String p("<p>");
    title.replace(QLatin1String(" - "), p);
    title.replace(QLatin1String(" | "), p);
    title.replace(QLatin1String(" — "), p);
    title.replace(QLatin1String(": "), p);
    title.replace(QLatin1String("; "), p);
    title.replace(QLatin1String("] "), QLatin1String("]<p>"));
    title.replace(QLatin1String(" ["), QLatin1String("<p>["));
    title.replace(QLatin1String(" ("), QLatin1String("<p>("));
    title.replace(QLatin1String(") "), QLatin1String(")<p>"));
    titleLabel->setText(title);
    titleLabel->setVisible(window()->height() > 100);

    const int maxDescLength = 500;

    QString videoDesc = video->getDescription();
    if (videoDesc.length() > maxDescLength) {
        videoDesc.truncate(maxDescLength);
        videoDesc = videoDesc.trimmed();
        videoDesc.append("…");
    } else if (videoDesc.endsWith(QLatin1String(" ..."))) {
        videoDesc = videoDesc.left(videoDesc.length() - 4);
        videoDesc.append("…");
    }
    static const QRegularExpression linkRE("(https?://\\S+)");
    videoDesc.replace(linkRE, QStringLiteral("<a style='color:white' href=\"\\1\">\\1</a>"));
    descriptionLabel->setText(videoDesc);
    bool hiddenDesc = height() < 400;
    if (hiddenDesc)
        titleLabel->setAlignment(Qt::AlignCenter);
    else
        titleLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    descriptionLabel->setVisible(!hiddenDesc);

    progressBar->setValue(0);
    startTime.start();
}

void LoadingWidget::setError(const QString &message) {
    titleLabel->setText(tr("Error"));
    descriptionLabel->setText(message);
    progressBar->setValue(0);
}

void LoadingWidget::bufferStatus(qreal value) {
    int percent = value * 100.;
    if (startTime.elapsed() > 1000 && percent > progressBar->value())
        progressBar->setValue(percent);
}

void LoadingWidget::adjustFontSize() {
    QFont f = titleLabel->font();
    int smallerDimension = qMin(height(), width());
    f.setPixelSize(smallerDimension / 12);
    QFontMetrics fm(f);
    int textHeightInPixels = fm.height();
    int spacing = textHeightInPixels / 2;
    layout()->setSpacing(spacing);
    layout()->setContentsMargins(spacing, spacing, spacing, spacing);
    titleLabel->setFont(f);

    QFont descFont = descriptionLabel->font();
    descFont.setPixelSize(f.pixelSize() / 2);
    descriptionLabel->setFont(descFont);
}

void LoadingWidget::clear() {
    titleLabel->clear();
    descriptionLabel->clear();
    progressBar->setValue(0);
}

void LoadingWidget::resizeEvent(QResizeEvent *e) {
    Q_UNUSED(e);
    if (isVisible()) adjustFontSize();
}
