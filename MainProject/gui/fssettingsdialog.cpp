/*****************************************************************************
 *
 * This file is part of the 'WebCamForceSettings' project.
 *
 * This project is designed to work with the settings (properties) of the
 * webcam, contains programs that are covered by one license.
 *
 * Copyright (C) 2021 Alexander Tsyganov.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
*****************************************************************************/

#include "fssettingsdialog.h"
#include "ui_fssettingsdialog.h"

#include <QHash>
#include <QLocale>
#include <QPushButton>

#include <WebCamFS/Settings>
#include <WebCamFS/TranslationsHelper>

#ifdef BUILD_WITH_Q_EASY_SETTINGS
#include "qeasysettings.hpp"

#define AUTO_STYLE_NAME         tr("Auto")
#define VISTA_STYLE_NAME        tr("Vista")
#define CLASSIC_STYLE_NAME      tr("Classic")
#define LIGHT_FUSION_STYLE_NAME tr("LightFusion")
#define DARK_FUSION_STYLE_NAME  tr("DarkFusion")
#endif // BUILD_WITH_Q_EASY_SETTINGS

class FSSettingsDialogPrivate
{
public:
    FSSettingsDialogPrivate();

    QHash<QString, QLocale> hashItemTextLocales;
};

FSSettingsDialogPrivate::FSSettingsDialogPrivate()
{

}

FSSettingsDialog::FSSettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FSSettingsDialog)
{
    init();
}

FSSettingsDialog::~FSSettingsDialog()
{
    delete ui;

    delete d;
    d = nullptr;
}

void FSSettingsDialog::setCurrentLocale(const QLocale &locale)
{
    ui->comboBoxLanguage->setCurrentIndex(ui->comboBoxLanguage->findText(genLocaleName(locale)));
}

QLocale FSSettingsDialog::currentLocale() const
{
    if (ui->comboBoxLanguage->currentIndex() != -1)
        return d->hashItemTextLocales.value(ui->comboBoxLanguage->itemText(ui->comboBoxLanguage->currentIndex()));
    else
        return QLocale::system();
}

void FSSettingsDialog::updateLocales()
{
    d->hashItemTextLocales.clear();
    ui->comboBoxLanguage->clear();

    QStringList items;

    QList<QLocale> locales = fsTH->availableFSLocales();
    d->hashItemTextLocales.reserve(locales.count());

    foreach (const QLocale &locale, locales) {
        const QString itemText = genLocaleName(locale);
        d->hashItemTextLocales.insert(itemText, locale);
        items.push_back(itemText);
    }

    items.sort();

    ui->comboBoxLanguage->addItems(items);

    ui->comboBoxLanguage->setEnabled(!locales.isEmpty());

    updateCurrentLanguage();
}

void FSSettingsDialog::setAutoStartEnable(bool isEnable)
{
    ui->checkBoxAutoStart->setEnabled(isEnable);
}

bool FSSettingsDialog::isAutoStartEnable() const
{
    return ui->checkBoxAutoStart->isEnabled();
}

void FSSettingsDialog::setAutoStart(bool isAutoStart)
{
    ui->checkBoxAutoStart->setChecked(isAutoStart);
}

bool FSSettingsDialog::isAutoStart() const
{
    return ui->checkBoxAutoStart->isChecked();
}

void FSSettingsDialog::setCustomStyleIndex(int styleIndex)
{
    ui->comboBoxStyle->setCurrentIndex(ui->comboBoxStyle->findText(getCustomStyleName(styleIndex)));
}

int FSSettingsDialog::customStyleIndex() const
{
    return getCustomStyleIndex(ui->comboBoxStyle->itemText(ui->comboBoxStyle->currentIndex()));
}

void FSSettingsDialog::setCameraDetectionEnable(bool isEnable)
{
    ui->checkBoxIsCameraDetectionEnable->setChecked(isEnable);
}

bool FSSettingsDialog::isCameraDetectionEnable() const
{
    return ui->checkBoxIsCameraDetectionEnable->isChecked();
}

void FSSettingsDialog::setCameraDetectionInterval(int msec)
{
    ui->spinBoxCameraDetectionInterval->setValue(msec);
}

int FSSettingsDialog::cameraDetectionInterval() const
{
    return ui->spinBoxCameraDetectionInterval->value();
}

void FSSettingsDialog::setCameraValueUpdateEnable(bool isEnable)
{
    ui->checkBoxIsCameraValueUpdateEnable->setChecked(isEnable);
}

bool FSSettingsDialog::isCameraValueUpdateEnable() const
{
    return ui->checkBoxIsCameraValueUpdateEnable->isChecked();
}

void FSSettingsDialog::setCameraValueUpdateInterval(int msec)
{
    ui->spinBoxCameraValueUpdateInterval->setValue(msec);
}

int FSSettingsDialog::cameraValueUpdateInterval() const
{
    return ui->spinBoxCameraValueUpdateInterval->value();
}

void FSSettingsDialog::setLockPropertiesEnable(bool isEnable)
{
    ui->checkBoxIsLockProperties->setChecked(isEnable);
}

bool FSSettingsDialog::isLockPropertiesEnable() const
{
    return ui->checkBoxIsLockProperties->isChecked();
}

void FSSettingsDialog::setLockPropertiesInterval(int msec)
{
    ui->spinBoxLockPropertiesInterval->setValue(msec);
}

int FSSettingsDialog::lockPropertiesInterval() const
{
    return ui->spinBoxLockPropertiesInterval->value();
}

void FSSettingsDialog::init()
{
    d = new FSSettingsDialogPrivate();

    ui->setupUi(this);

    connect(fsTH, &FSTranslationsHelper::currentLanguageChanged,
            this, &FSSettingsDialog::retranslate);

    updateLocales();

    connect(fsTH, &FSTranslationsHelper::currentLanguageChanged,
            this, &FSSettingsDialog::updateCurrentLanguage);

    const QStringList customStyleNames = getCustomStyleNames();
    if (!customStyleNames.isEmpty()) {
        ui->comboBoxStyle->addItems(customStyleNames);
    } else {
        ui->labelStyle->setEnabled(false);
        ui->comboBoxStyle->setEnabled(false);
    }

    QPushButton *restoreDefaultsButton = ui->buttonBox->button(QDialogButtonBox::RestoreDefaults);
    connect(restoreDefaultsButton, SIGNAL(clicked(bool)),
            this,                    SLOT(restoreDefaultValues()));

    connectionCheckBoxWithLabelAndSpinBox(ui->checkBoxIsCameraDetectionEnable,
                                          ui->labelCameraDetectionInterval,
                                          ui->spinBoxCameraDetectionInterval);

    connectionCheckBoxWithLabelAndSpinBox(ui->checkBoxIsCameraValueUpdateEnable,
                                          ui->labelCameraValueUpdateInterval,
                                          ui->spinBoxCameraValueUpdateInterval);

    connectionCheckBoxWithLabelAndSpinBox(ui->checkBoxIsLockProperties,
                                          ui->labelLockPropertiesInterval,
                                          ui->spinBoxLockPropertiesInterval);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

void FSSettingsDialog::connectionCheckBoxWithLabelAndSpinBox(QCheckBox *checkBox,
                                                             QLabel *label,
                                                             QSpinBox *spinBox)
{
    connect(checkBox, &QCheckBox::toggled,
            label,    &QWidget::setEnabled);
    connect(checkBox, &QCheckBox::toggled,
            spinBox,  &QWidget::setEnabled);

    label->setEnabled(false);
    spinBox->setEnabled(false);
}

QString FSSettingsDialog::genLocaleName(const QLocale &locale)
{
    return QString("%1 (%2)").arg(QLocale::languageToString(locale.language()), locale.nativeLanguageName());
}

QStringList FSSettingsDialog::getCustomStyleNames()
{
#ifdef BUILD_WITH_Q_EASY_SETTINGS
    return { AUTO_STYLE_NAME, VISTA_STYLE_NAME, CLASSIC_STYLE_NAME, LIGHT_FUSION_STYLE_NAME, DARK_FUSION_STYLE_NAME };
#else
    return QStringList();
#endif // BUILD_WITH_Q_EASY_SETTINGS
}

QString FSSettingsDialog::getCustomStyleName(int styleIndex)
{
#ifdef BUILD_WITH_Q_EASY_SETTINGS
    switch (styleIndex) {
    case int(QEasySettings::Style::autoFusion):
        return AUTO_STYLE_NAME;
    case int(QEasySettings::Style::vista):
        return VISTA_STYLE_NAME;
    case int(QEasySettings::Style::classic):
        return CLASSIC_STYLE_NAME;
    case int(QEasySettings::Style::lightFusion):
        return LIGHT_FUSION_STYLE_NAME;
    case int(QEasySettings::Style::darkFusion):
        return DARK_FUSION_STYLE_NAME;
    default:
        break;
    }
#else
    Q_UNUSED(styleIndex);
#endif // BUILD_WITH_Q_EASY_SETTINGS

    return QString();
}

int FSSettingsDialog::getCustomStyleIndex(const QString &styleName)
{
#ifdef BUILD_WITH_Q_EASY_SETTINGS
    if (styleName == AUTO_STYLE_NAME) {
        return int(QEasySettings::Style::autoFusion);
    } else if (styleName == VISTA_STYLE_NAME) {
        return int(QEasySettings::Style::vista);
    } else if (styleName == CLASSIC_STYLE_NAME) {
        return int(QEasySettings::Style::classic);
    } else if (styleName == LIGHT_FUSION_STYLE_NAME) {
        return int(QEasySettings::Style::lightFusion);
    } else if (styleName == DARK_FUSION_STYLE_NAME) {
        return int(QEasySettings::Style::darkFusion);
    }
#else
    Q_UNUSED(styleName);
#endif // BUILD_WITH_Q_EASY_SETTINGS

    return -1;
}

void FSSettingsDialog::retranslate()
{
    ui->retranslateUi(this);
}

void FSSettingsDialog::restoreDefaultValues()
{
    updateCurrentLanguage();

    if (isAutoStartEnable())
        setAutoStart(true);

    setCameraDetectionEnable(FSSettings::defaultIsCameraDetectionEnable());
    setCameraDetectionInterval(FSSettings::defaultCameraDetectionInterval());
    setCameraValueUpdateEnable(FSSettings::defaultIsCameraValueUpdateEnable());
    setCameraValueUpdateInterval(FSSettings::defaultCameraValueUpdateInterval());
    setLockPropertiesEnable(FSSettings::defaultIsLockPropertiesEnable());
    setLockPropertiesInterval(FSSettings::defaultLockPropertiesInterval());
}

void FSSettingsDialog::updateCurrentLanguage()
{
    setCurrentLocale(fsTH->currentLocale());
}
