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

    titleLabel = new QLabel(this);
    titleLabel->setPalette(p);
    titleLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    titleLabel->setWordWrap(true);
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    titleLabel->setFont(FontUtils::light(titleLabel->font().pointSize()));
    layout->addWidget(titleLabel);

    descriptionLabel = new QLabel(this);
    descriptionLabel->setPalette(p);
    descriptionLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
    descriptionLabel->setTextFormat(Qt::RichText);
    descriptionLabel->setOpenExternalLinks(true);
    layout->addWidget(descriptionLabel);

    progressBar = new QProgressBar(this);
    progressBar->setAutoFillBackground(false);
    progressBar->setPalette(p);
    // progressBar->hide();
    progressBar->setStyleSheet("QProgressBar {max-height:3px; background:black; border:0} "
                               "QProgressBar::chunk {background:white}");
    progressBar->setTextVisible(false);
    layout->addWidget(progressBar);
}

void LoadingWidget::setVideo(Video *video) {
    adjustFontSize();

    QString title = video->getTitle();
    // enhance legibility by splitting the title
    title.replace(QLatin1String(" - "), QLatin1String("<p>"));
    title.replace(QLatin1String(" | "), QLatin1String("<p>"));
    title.replace(QLatin1String(" — "), QLatin1String("<p>"));
    title.replace(QLatin1String("] "), QLatin1String("]<p>"));
    title.replace(QLatin1String(" ["), QLatin1String("<p>["));
    title.replace(QLatin1String(" ("), QLatin1String("<p>("));
    title.replace(QLatin1String(") "), QLatin1String(")<p>"));
    titleLabel->setText(title);
    titleLabel->setVisible(window()->height() > 100);

    static const int maxDescLength = 400;
    QString videoDesc = video->getDescription();
    if (videoDesc.length() > maxDescLength) {
        videoDesc.truncate(maxDescLength);
        videoDesc = videoDesc.trimmed();
        videoDesc.append("…");
    } else if (videoDesc.endsWith(QLatin1String(" ..."))) {
        videoDesc = videoDesc.left(videoDesc.length() - 4);
        videoDesc.append("…");
    }
    videoDesc.replace(QRegExp("(https?://\\S+)"), "<a style='color:white' href=\"\\1\">\\1</a>");
    descriptionLabel->setText(videoDesc);
    bool hiddenDesc = height() < 400;
    if (hiddenDesc)
        titleLabel->setAlignment(Qt::AlignCenter);
    else
        titleLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    descriptionLabel->setVisible(!hiddenDesc);

    // progressBar->hide();
    progressBar->setValue(0);
    startTime.start();
}

void LoadingWidget::setError(const QString &message) {
    titleLabel->setText(tr("Error"));
    descriptionLabel->setText(message);
    // progressBar->hide();
    progressBar->setValue(0);
}

void LoadingWidget::bufferStatus(int percent) {
    /*
    if (progressBar->isHidden() && percent > 0) {
        progressBar->show();
        QPropertyAnimation *animation = new QPropertyAnimation(progressBar, "opacity");
        animation->setDuration(1000);
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);
        animation->start();
    }*/
    if (startTime.elapsed() < 2000 || percent <= progressBar->value()) return;
    // progressBar->setShown(percent > 0);
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
    layout()->setMargin(spacing);
    titleLabel->setFont(f);

    f.setPixelSize(f.pixelSize() / 2);
    descriptionLabel->setFont(f);
}

void LoadingWidget::clear() {
    titleLabel->clear();
    descriptionLabel->clear();
    // progressBar->hide();
    progressBar->setValue(0);
}

void LoadingWidget::resizeEvent(QResizeEvent *e) {
    Q_UNUSED(e);
    if (isVisible()) adjustFontSize();
}
