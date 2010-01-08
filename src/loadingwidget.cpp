#include "loadingwidget.h"

LoadingWidget::LoadingWidget(QWidget *parent) : QWidget(parent) {

    QPalette p = palette();
    p.setBrush(QPalette::Window, Qt::black);
    p.setBrush(QPalette::Text, Qt::white);
    setPalette(p);

    setAutoFillBackground(true);

    QFont bigFont;
    bigFont.setPointSize(bigFont.pointSize()*4);
    QFontMetrics fm(bigFont);
    int textHeightInPixels = fm.height();
    int spacing = textHeightInPixels / 2;

    QBoxLayout *layout = new QVBoxLayout();
    layout->setSpacing(spacing);
    layout->setMargin(spacing);

    titleLabel = new QLabel(this);
    titleLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    titleLabel->setPalette(p);
    titleLabel->setForegroundRole(QPalette::Text);
    titleLabel->setWordWrap(true);
    titleLabel->setFont(bigFont);
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(titleLabel);

    QFont biggerFont;
    biggerFont.setPointSize(biggerFont.pointSize()*2);

    descriptionLabel = new QLabel(this);
    descriptionLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    descriptionLabel->setPalette(p);
    descriptionLabel->setForegroundRole(QPalette::Text);
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setFont(biggerFont);
    descriptionLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(descriptionLabel);

    /*
    progressBar = new QProgressBar(this);
    progressBar->hide();
    layout->addWidget(progressBar);
    */

    setMouseTracking(true);

    setLayout(layout);
}

void LoadingWidget::setVideo(Video *video) {
    QString title = video->title();
    // enhance legibility by splitting the title
    title = title.replace(" - ", "<p>");
    title = title.replace("] ", "]<p>");
    title = title.replace(" [", "<p>[");
    titleLabel->setText(title);
    descriptionLabel->setText(video->description());
    // progressBar->hide();
}

void LoadingWidget::setError(QString message) {
    titleLabel->setText(tr("Error"));
    descriptionLabel->setText(message);
    // progressBar->hide();
}

void LoadingWidget::bufferStatus(int percent) {
    /*
    qDebug() << percent;
    progressBar->setShown(percent > 0);
    progressBar->setValue(percent);
    */
}

void LoadingWidget::clear() {
    titleLabel->clear();
    descriptionLabel->clear();
    // progressBar->hide();
}
