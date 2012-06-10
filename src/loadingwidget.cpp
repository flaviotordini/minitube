#include "loadingwidget.h"

LoadingWidget::LoadingWidget(QWidget *parent) : QWidget(parent) {

    QPalette p = palette();
    p.setBrush(backgroundRole(), Qt::black);
    p.setBrush(QPalette::Text, Qt::white);
    setPalette(p);

    setAutoFillBackground(true);

    QBoxLayout *layout = new QVBoxLayout();

    titleLabel = new QLabel(this);
    titleLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    titleLabel->setPalette(p);
    titleLabel->setForegroundRole(QPalette::Text);
    titleLabel->setWordWrap(true);
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(titleLabel);

    QFont biggerFont;
    biggerFont.setPointSize(biggerFont.pointSize()*2);

    descriptionLabel = new QLabel(this);
    descriptionLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    descriptionLabel->setPalette(p);
    descriptionLabel->setForegroundRole(QPalette::Text);
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
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

    setLayout(layout);
}

void LoadingWidget::setVideo(Video *video) {

    QFont titleFont;
    int smallerDimension = qMin(height(), width());
    titleFont.setPixelSize(smallerDimension / 12);
    QFontMetrics fm(titleFont);
    int textHeightInPixels = fm.height();
    int spacing = textHeightInPixels / 2;
    layout()->setSpacing(spacing);
    layout()->setMargin(spacing);

    QString title = video->title();
    // enhance legibility by splitting the title
    title = title.replace(" - ", "<p>");
    title = title.replace("] ", "]<p>");
    title = title.replace(" [", "<p>[");
    titleLabel->setText(title);
    titleLabel->setVisible(window()->height() > 100);
    titleLabel->setFont(titleFont);

    static const int maxVideoLength = 256;
    QString videoDesc = video->description();
    if (videoDesc.length() > maxVideoLength) {
        videoDesc.truncate(maxVideoLength-1);
        videoDesc.append("...");
    }
    QFont descFont(titleFont);
    descFont.setPixelSize(descFont.pixelSize() / 2);
    descriptionLabel->setFont(descFont);
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

void LoadingWidget::setError(QString message) {
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

void LoadingWidget::clear() {
    titleLabel->clear();
    descriptionLabel->clear();
    // progressBar->hide();
    progressBar->setValue(0);
}
