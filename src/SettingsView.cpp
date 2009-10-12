#include "SettingsView.h"

SettingsView::SettingsView( QWidget *parent ) : QWidget(parent) {

    /*
    QSettings settings;

    QBoxLayout *layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    // layout->setSpacing(30);
    
    QFormLayout *formLayout = new QFormLayout;
    formLayout->setFormAlignment(Qt::AlignHCenter | Qt::AlignTop);
    formLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

    QGroupBox *videoGroup = new QGroupBox(tr("&Video options"), this);

    contrastSlider = new QSlider(Qt::Horizontal, this);
    contrastSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    formLayout->addRow("&Contrast", contrastSlider);

    brightnessSlider = new QSlider(Qt::Horizontal, this);
    brightnessSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    formLayout->addRow("&Brightness", brightnessSlider);

    saturationSlider = new QSlider(Qt::Horizontal, this);
    saturationSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    formLayout->addRow("&Saturation", saturationSlider);

    hueSlider = new QSlider(Qt::Horizontal, this);
    hueSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    formLayout->addRow("&Hue", hueSlider);

    hdVideoCheckBox = new QCheckBox(tr("Use high quality video when available"), this);
    formLayout->addRow("", hdVideoCheckBox);

    maxRecentKeywordsSpinBox = new QSpinBox(this);
    maxRecentKeywordsSpinBox->setMinimum(1);
    maxRecentKeywordsSpinBox->setMaximum(30);
    // maxRecentKeywordsSpinBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    formLayout->addRow(tr("&Saved recent keywords"), maxRecentKeywordsSpinBox);
    layout->addLayout(formLayout);

    clearRecentKeywordsButton = new QPushButton(tr("&Clear recent keywords"), this);
    // clearRecentKeywordsButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    formLayout->addRow("", clearRecentKeywordsButton);

    QPushButton *closeButton = new QPushButton(tr("&Close"), this);
    // closeButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    closeButton->setDefault(true);
    closeButton->setFocus(Qt::OtherFocusReason);
    connect(closeButton, SIGNAL(clicked()), parent, SLOT(goBack()));
    formLayout->addRow("", closeButton);
    */

}

void SettingsView::storeSettings() {

}
