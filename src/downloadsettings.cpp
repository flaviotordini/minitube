#include "downloadsettings.h"
#include "downloadmanager.h"

DownloadSettings::DownloadSettings(QWidget *parent) : QWidget(parent) {

    QBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(10);

    message = new QLabel(this);
    message->setOpenExternalLinks(true);
    layout->addWidget(message);

    changeFolderButton = new QPushButton(this);
    changeFolderButton->setText(tr("Change location..."));
    changeFolderButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    connect(changeFolderButton, SIGNAL(clicked()), SLOT(changeFolder()));
    layout->addWidget(changeFolderButton);

    updateMessage();
}

void DownloadSettings::paintEvent(QPaintEvent * /*event*/) {
    QPainter painter(this);
#ifdef APP_MAC
    QBrush brush;
    if (window()->isActiveWindow()) {
        brush = QBrush(QColor(0xdd, 0xe4, 0xeb));
    } else {
        brush = palette().window();
    }
    painter.fillRect(0, 0, width(), height(), brush);
#endif
    painter.setPen(palette().color(QPalette::Mid));
    painter.drawLine(0, 0, width(), 0);
}

void DownloadSettings::changeFolder() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose the download location"),
                                                    QDesktopServices::storageLocation(QDesktopServices::HomeLocation),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!dir.isEmpty()) {
        QSettings settings;
        settings.setValue("downloadFolder", dir);
        updateMessage();
        QMainWindow* mainWindow = dynamic_cast<QMainWindow*>(window());
        if (mainWindow) {
            QString status;
            status = tr("Download location changed.");
            if (DownloadManager::instance()->activeItems() > 0)
                status += " " + tr("Current downloads will still go in the previous location.");
            mainWindow->statusBar()->showMessage(status);
        }
    }
}

void DownloadSettings::updateMessage() {
    QString path = DownloadManager::instance()->currentDownloadFolder();
    QString home = QDesktopServices::storageLocation(QDesktopServices::HomeLocation) + "/";
    QString displayPath = path;
    displayPath = displayPath.remove(home);
    message->setText(
            tr("Downloading to: %1")
            .arg("<a href='file://%1' style='text-decoration:none; color:palette(text); font-weight:bold'>%2</a>")
            .arg(path, displayPath));
}
