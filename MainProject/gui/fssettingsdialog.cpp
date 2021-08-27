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
