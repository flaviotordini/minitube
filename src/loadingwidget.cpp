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

LoadingWidget::LoadingWidget(QWidget *parent) : QWidget(parent) {

    QPalette p = palette();
    p.setBrush(backgroundRole(), Qt::black);
    p.setBrush(QPalette::Text, Qt::white);
    setPalette(p);

    setAutoFillBackground(true);

    QBoxLayout *layout = new QVBoxLayout(this);

    titleLabel = new QLabel(this);
    titleLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    titleLabel->setPalette(p);
    titleLabel->setForegroundRole(QPalette::Text);
    titleLabel->setWordWrap(true);
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(titleLabel);

    descriptionLabel = new QLabel(this);
    descriptionLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    descriptionLabel->setPalette(p);
    descriptionLabel->setForegroundRole(QPalette::Text);
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
    descriptionLabel->setTextFormat(Qt::RichText);
    descriptionLabel->setOpenExternalLinks(true);
    layout->addWidget(descriptionLabel);

    progressBar = new QProgressBar(this);
    progressBar->setAutoFillBackground(false);
    progressBar->setBackgroundRole(QPalette::Window);
    progressBar->setPalette(p);
    // progressBar->hide();
    progressBar->setStyleSheet("QProgressBar {max-height:3px; background:black; border:0} QProgressBar::chunk {background:white}");
    progressBar->setTextVisible(false);
    layout->addWidget(progressBar);

    setMouseTracking(true);
}

void LoadingWidget::setVideo(Video *video) {
    adjustFontSize();

    QString title = video->title();
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
    QString videoDesc = video->description();
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
    // qDebug() << percent;

    /*
    if (progressBar->isHidden() && percent > 0) {
        progressBar->show();
        QPropertyAnimation *animation = new QPropertyAnimation(progressBar, "opacity");
        animation->setDuration(1000);
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);
        animation->start();
    }*/
    // progressBar->setShown(percent > 0);
    if (startTime.elapsed() < 1000) return;
    if (progressBar->value() == 0 && percent > 80) return;
    progressBar->setValue(percent);
}

void LoadingWidget::adjustFontSize() {
    QFont titleFont;
#ifdef APP_MAC
    titleFont.setFamily("Helvetica Neue");
    titleFont.setStyleName("Thin");
#elif APP_WIN
    titleFont.setFamily("Segoe UI Light");
    titleFont.setStyleName("Light");
#else
    titleFont.setStyleName("Light");
#endif
    int smallerDimension = qMin(height(), width());
    titleFont.setPixelSize(smallerDimension / 12);
    QFontMetrics fm(titleFont);
    int textHeightInPixels = fm.height();
    int spacing = textHeightInPixels / 2;
    layout()->setSpacing(spacing);
    layout()->setMargin(spacing);
    titleLabel->setFont(titleFont);

    QFont descFont(titleFont);
    descFont.setPixelSize(descFont.pixelSize() / 2);
    descriptionLabel->setFont(descFont);
}

void LoadingWidget::clear() {
    titleLabel->clear();
    descriptionLabel->clear();
    // progressBar->hide();
    progressBar->setValue(0);
}

void LoadingWidget::resizeEvent(QResizeEvent *e) {
    if (isVisible()) adjustFontSize();
}
