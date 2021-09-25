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

#include "fscamerasettingsdialog.h"
#include "ui_fscamerasettingsdialog.h"

#include <deque>
#include <limits>

#include <QtMath>
#include <QTimer>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QApplication>
#include <QSignalMapper>

#include <WebCamFS/Settings>
#include <WebCamFS/CamerasStorage>
#include <WebCamFS/TranslationsHelper>
#include <WebCamFS/LockPropertiesManager>
#include <WebCamFS/RenamePresetNameDialog>

#define MAX_FS_CAMERA_FREE_PRESET_COUNT 1000
#define MAX_FS_CAMERA_PROPERTY_COUNT    1000

#define MANUAL_PRESET FSCameraSettingsDialog::tr("Manual")
#define STRING_PRESET FSCameraSettingsDialog::tr("Preset %1")
#define TR_NEW_PRESET FSCameraSettingsDialog::tr("<New preset>")

#define STRING_POWERLINE_FREQUENCY_50 FSCameraSettingsDialog::tr("50 Hz")
#define STRING_POWERLINE_FREQUENCY_60 FSCameraSettingsDialog::tr("60 Hz")

QString fsGetTrEnumName(FSCameraProperty property) {
    switch (property) {
    case FSCameraProperty::None:
        return QString();
    case FSCameraProperty::Brightness:
        return FSCameraSettingsDialog::tr("Brightness");
    case FSCameraProperty::Contrast:
        return FSCameraSettingsDialog::tr("Contrast");
    case FSCameraProperty::Hue:
        return FSCameraSettingsDialog::tr("Hue");
    case FSCameraProperty::Saturation:
        return FSCameraSettingsDialog::tr("Saturation");
    case FSCameraProperty::Sharpness:
        return FSCameraSettingsDialog::tr("Sharpness");
    case FSCameraProperty::Gamma:
        return FSCameraSettingsDialog::tr("Gamma");
    case FSCameraProperty::ColorEnable:
        return FSCameraSettingsDialog::tr("Color enable");
    case FSCameraProperty::WhiteBalance:
        return FSCameraSettingsDialog::tr("White balance");
    case FSCameraProperty::BacklightCompensation:
        return FSCameraSettingsDialog::tr("Backlight compensation");
    case FSCameraProperty::Gain:
        return FSCameraSettingsDialog::tr("Gain");
    case FSCameraProperty::PowerlineFrequency:
        return FSCameraSettingsDialog::tr("Powerline frequency");
    case FSCameraProperty::Pan:
        return FSCameraSettingsDialog::tr("Pan");
    case FSCameraProperty::Tilt:
        return FSCameraSettingsDialog::tr("Tilt");
    case FSCameraProperty::Roll:
        return FSCameraSettingsDialog::tr("Roll");
    case FSCameraProperty::Zoom:
        return FSCameraSettingsDialog::tr("Zoom");
    case FSCameraProperty::Exposure:
        return FSCameraSettingsDialog::tr("Exposure");
    case FSCameraProperty::Iris:
        return FSCameraSettingsDialog::tr("Iris");
    case FSCameraProperty::Focus:
        return FSCameraSettingsDialog::tr("Focus");
    }

    return QString();
}

class FSCameraSettingsDialogPrivate
{
public:
    FSCameraSettingsDialogPrivate();

    FSCamera *camera;

    QString origWindowTitle;

    FSCameraSettingsDialog::Mode mode;

    QTimer *valueUpdateTimer;

    bool isCameraValueSendEnable;

    QSignalMapper *propertyChangeSignalMapper;

    FSLockPropertiesManager *lockPropertiesManager;
    QSignalMapper *lockSignalMapper;

    FSCameraPropertyRangesUMap umapCameraPropertyRangeParams;

    QString currentPresetName;
    FSCameraUserPresetsUMap umapCameraUserPresets;

    static QLabel    *getLabel(Ui::FSCameraSettingsDialog *ui, FSCameraProperty property);
    static QSlider   *getSlider(Ui::FSCameraSettingsDialog *ui, FSCameraProperty property);
    static QSpinBox  *getSpinBox(Ui::FSCameraSettingsDialog *ui, FSCameraProperty property);
    static QCheckBox *getAutoCheckBox(Ui::FSCameraSettingsDialog *ui, FSCameraProperty property);
    static QCheckBox *getLockCheckBox(Ui::FSCameraSettingsDialog *ui, FSCameraProperty property);

    static int valueForSlider(int value, const FSRangeParams &rangeParams);
    static int valueForSpinBox(int value, const FSRangeParams &rangeParams);

    static int valueFromSlider(int value, const FSRangeParams &rangeParams);
    static int valueFromSpinBox(int value, const FSRangeParams &rangeParams);

    void setEditorsRange(Ui::FSCameraSettingsDialog *ui,
                         FSCameraProperty property,
                         const FSRangeParams &rangeParams) const;

    static void setLabelRangeParams(QLabel *label, const FSRangeParams &rangeParams);
    static void setSliderRangeParams(QSlider *slider, const FSRangeParams &rangeParams, bool isLock);
    static void setSpinBoxRangeParams(QSpinBox *spinBox, const FSRangeParams &rangeParams, bool isLock);
    static void setAutoCheckBoxRangeParams(QCheckBox *autoCheckBox, const FSRangeParams &rangeParams, bool isLock);
    static void setLockCheckBoxRangeParams(QCheckBox *lockCheckBox, const FSRangeParams &rangeParams);

    static void setCheckBoxColorEnableRangeParams(Ui::FSCameraSettingsDialog *ui,
                                                  const FSRangeParams &rangeParams,
                                                  bool isLock);
    static void setComboBoxPowerlineFrequencyRangeParams(Ui::FSCameraSettingsDialog *ui,
                                                         const FSRangeParams &rangeParams,
                                                         bool isLock);

    void setEditorsValue(Ui::FSCameraSettingsDialog *ui,
                         FSCameraProperty property,
                         const FSValueParams &valueParams) const;

    static void setLabelSelection(QLabel *label, const FSValueParams &valueParams, const FSValueParams &defaultParams, bool isLock);
    static void setSliderValueParams(QSlider *slider, const FSValueParams &valueParams, const FSRangeParams &rangeParams, bool isLock);
    static void setSpinBoxValueParams(QSpinBox *spinBox, const FSValueParams &valueParams, const FSRangeParams &rangeParams, bool isLock);
    static void setAutoCheckBoxValueParams(QCheckBox *autoCheckBox, const FSValueParams &valueParams, const FSRangeParams &rangeParams, bool isLock);

    static void setCheckBoxColorEnableValueParams(Ui::FSCameraSettingsDialog *ui,
                                                  const FSValueParams &valueParams,
                                                  bool isLock);
    static void setComboBoxPowerlineFrequencyValueParams(Ui::FSCameraSettingsDialog *ui,
                                                         const FSValueParams &valueParams,
                                                         bool isLock);

    bool isEditorValueEnable(FSCameraProperty property) const;

    enum EditorType {
        NoneEditorType = 0,
        SliderEditorType,
        SpinBoxEditorType,
        AutoCheckBoxEditorType
    };

    FSRangeParams getRangeParams(FSCameraProperty property) const;

    FSValueParams getActualValue(FSCameraProperty property) const;
    FSValueParams getDefaultValue(FSCameraProperty property) const;
    FSValueParams getDefaultValueFromRange(FSCameraProperty property) const;

    FSValueParams getEditorValue(Ui::FSCameraSettingsDialog *ui, FSCameraProperty property, EditorType editorType = NoneEditorType) const;

    FSValueParams standartValueParams(Ui::FSCameraSettingsDialog *ui, EditorType editorType, FSCameraProperty property) const;
    FSValueParams colorEnableValueParams(Ui::FSCameraSettingsDialog *ui) const;
    FSValueParams powerlineFrequencyValueParams(Ui::FSCameraSettingsDialog *ui) const;

    void connectingEditors(Ui::FSCameraSettingsDialog *ui, FSCameraProperty property);

    void connectingStandartEditors(Ui::FSCameraSettingsDialog *ui, FSCameraProperty property);
    void connectingColorEnableEditor(Ui::FSCameraSettingsDialog *ui);
    void connectingPowerlineFrequencyEditor(Ui::FSCameraSettingsDialog *ui);

    void connectingLockCheckBox(Ui::FSCameraSettingsDialog *ui, FSCameraProperty property);

    void readRangesFromDevice(Ui::FSCameraSettingsDialog *ui, FSCameraProperty property);
    void updateValue(Ui::FSCameraSettingsDialog *ui, FSCameraProperty property);

    static int genEditorTypeProperty(EditorType editorType, FSCameraProperty property);
    static EditorType getEditorType(int editorTypeProperty);
    static FSCameraProperty getProperty(int editorTypeProperty);

    void sendValueParams(Ui::FSCameraSettingsDialog *ui, FSCameraProperty property, EditorType editorType);
};

FSCameraSettingsDialogPrivate::FSCameraSettingsDialogPrivate()
    : camera(nullptr)
    , mode(FSCameraSettingsDialog::LockPropertiesMode)
    , valueUpdateTimer(nullptr)
    , isCameraValueSendEnable(true)
    , propertyChangeSignalMapper(nullptr)
    , lockPropertiesManager(nullptr)
    , lockSignalMapper(nullptr)
{
    umapCameraPropertyRangeParams.reserve(fsCameraPropertiesCount());
}

QLabel *FSCameraSettingsDialogPrivate::getLabel(Ui::FSCameraSettingsDialog *ui,
                                                FSCameraProperty property)
{
    switch (property) {
    case FSCameraProperty::None:
        return nullptr;
    case FSCameraProperty::Brightness:
        return ui->labelBrightness;
    case FSCameraProperty::Contrast:
        return ui->labelContrast;
    case FSCameraProperty::Hue:
        return ui->labelHue;
    case FSCameraProperty::Saturation:
        return ui->labelSaturation;
    case FSCameraProperty::Sharpness:
        return ui->labelSharpness;
    case FSCameraProperty::Gamma:
        return ui->labelGamma;
    case FSCameraProperty::ColorEnable:
        return ui->labelColorEnable;
    case FSCameraProperty::WhiteBalance:
        return ui->labelWhiteBalance;
    case FSCameraProperty::BacklightCompensation:
        return ui->labelBacklightCompensation;
    case FSCameraProperty::Gain:
        return ui->labelGain;
    case FSCameraProperty::PowerlineFrequency:
        return ui->labelPowerlineFrequency;
    case FSCameraProperty::Pan:
        return ui->labelPan;
    case FSCameraProperty::Tilt:
        return ui->labelTilt;
    case FSCameraProperty::Roll:
        return ui->labelRoll;
    case FSCameraProperty::Zoom:
        return ui->labelZoom;
    case FSCameraProperty::Exposure:
        return ui->labelExposure;
    case FSCameraProperty::Iris:
        return ui->labelIris;
    case FSCameraProperty::Focus:
        return ui->labelFocus;
    }

    return nullptr;
}

QSlider *FSCameraSettingsDialogPrivate::getSlider(Ui::FSCameraSettingsDialog *ui,
                                                  FSCameraProperty property)
{
    switch (property) {
    case FSCameraProperty::None:
        return nullptr;
    case FSCameraProperty::Brightness:
        return ui->horizontalSliderBrightness;
    case FSCameraProperty::Contrast:
        return ui->horizontalSliderContrast;
    case FSCameraProperty::Hue:
        return ui->horizontalSliderHue;
    case FSCameraProperty::Saturation:
        return ui->horizontalSliderSaturation;
    case FSCameraProperty::Sharpness:
        return ui->horizontalSliderSharpness;
    case FSCameraProperty::Gamma:
        return ui->horizontalSliderGamma;
    case FSCameraProperty::ColorEnable:
        return nullptr;
    case FSCameraProperty::WhiteBalance:
        return ui->horizontalSliderWhiteBalance;
    case FSCameraProperty::BacklightCompensation:
        return ui->horizontalSliderBacklightCompensation;
    case FSCameraProperty::Gain:
        return ui->horizontalSliderGain;
    case FSCameraProperty::PowerlineFrequency:
        return nullptr;
    case FSCameraProperty::Pan:
        return ui->horizontalSliderPan;
    case FSCameraProperty::Tilt:
        return ui->horizontalSliderTilt;
    case FSCameraProperty::Roll:
        return ui->horizontalSliderRoll;
    case FSCameraProperty::Zoom:
        return ui->horizontalSliderZoom;
    case FSCameraProperty::Exposure:
        return ui->horizontalSliderExposure;
    case FSCameraProperty::Iris:
        return ui->horizontalSliderIris;
    case FSCameraProperty::Focus:
        return ui->horizontalSliderFocus;
    }

    return nullptr;
}

QSpinBox *FSCameraSettingsDialogPrivate::getSpinBox(Ui::FSCameraSettingsDialog *ui,
                                                    FSCameraProperty property)
{
    switch (property) {
    case FSCameraProperty::None:
        return nullptr;
    case FSCameraProperty::Brightness:
        return ui->spinBoxBrightness;
    case FSCameraProperty::Contrast:
        return ui->spinBoxContrast;
    case FSCameraProperty::Hue:
        return ui->spinBoxHue;
    case FSCameraProperty::Saturation:
        return ui->spinBoxSaturation;
    case FSCameraProperty::Sharpness:
        return ui->spinBoxSharpness;
    case FSCameraProperty::Gamma:
        return ui->spinBoxGamma;
    case FSCameraProperty::ColorEnable:
        return nullptr;
    case FSCameraProperty::WhiteBalance:
        return ui->spinBoxWhiteBalance;
    case FSCameraProperty::BacklightCompensation:
        return ui->spinBoxBacklightCompensation;
    case FSCameraProperty::Gain:
        return ui->spinBoxGain;
    case FSCameraProperty::PowerlineFrequency:
        return nullptr;
    case FSCameraProperty::Pan:
        return ui->spinBoxPan;
    case FSCameraProperty::Tilt:
        return ui->spinBoxTilt;
    case FSCameraProperty::Roll:
        return ui->spinBoxRoll;
    case FSCameraProperty::Zoom:
        return ui->spinBoxZoom;
    case FSCameraProperty::Exposure:
        return ui->spinBoxExposure;
    case FSCameraProperty::Iris:
        return ui->spinBoxIris;
    case FSCameraProperty::Focus:
        return ui->spinBoxFocus;
    }

    return nullptr;
}

QCheckBox *FSCameraSettingsDialogPrivate::getAutoCheckBox(Ui::FSCameraSettingsDialog *ui,
                                                          FSCameraProperty property)
{
    switch (property) {
    case FSCameraProperty::None:
        return nullptr;
    case FSCameraProperty::Brightness:
        return ui->checkBoxAutoBrightness;
    case FSCameraProperty::Contrast:
        return ui->checkBoxAutoContrast;
    case FSCameraProperty::Hue:
        return ui->checkBoxAutoHue;
    case FSCameraProperty::Saturation:
        return ui->checkBoxAutoSaturation;
    case FSCameraProperty::Sharpness:
        return ui->checkBoxAutoSharpness;
    case FSCameraProperty::Gamma:
        return ui->checkBoxAutoGamma;
    case FSCameraProperty::ColorEnable:
        return nullptr;
    case FSCameraProperty::WhiteBalance:
        return ui->checkBoxAutoWhiteBalance;
    case FSCameraProperty::BacklightCompensation:
        return ui->checkBoxAutoBacklightCompensation;
    case FSCameraProperty::Gain:
        return ui->checkBoxAutoGain;
    case FSCameraProperty::PowerlineFrequency:
        return nullptr;
    case FSCameraProperty::Pan:
        return ui->checkBoxAutoPan;
    case FSCameraProperty::Tilt:
        return ui->checkBoxAutoTilt;
    case FSCameraProperty::Roll:
        return ui->checkBoxAutoRoll;
    case FSCameraProperty::Zoom:
        return ui->checkBoxAutoZoom;
    case FSCameraProperty::Exposure:
        return ui->checkBoxAutoExposure;
    case FSCameraProperty::Iris:
        return ui->checkBoxAutoIris;
    case FSCameraProperty::Focus:
        return ui->checkBoxAutoFocus;
    }

    return nullptr;
}

QCheckBox *FSCameraSettingsDialogPrivate::getLockCheckBox(Ui::FSCameraSettingsDialog *ui,
                                                          FSCameraProperty property)
{
    switch (property) {
    case FSCameraProperty::None:
        return nullptr;
    case FSCameraProperty::Brightness:
        return ui->checkBoxLockBrightness;
    case FSCameraProperty::Contrast:
        return ui->checkBoxLockContrast;
    case FSCameraProperty::Hue:
        return ui->checkBoxLockHue;
    case FSCameraProperty::Saturation:
        return ui->checkBoxLockSaturation;
    case FSCameraProperty::Sharpness:
        return ui->checkBoxLockSharpness;
    case FSCameraProperty::Gamma:
        return ui->checkBoxLockGamma;
    case FSCameraProperty::ColorEnable:
        return ui->checkBoxLockColorEnable;
    case FSCameraProperty::WhiteBalance:
        return ui->checkBoxLockWhiteBalance;
    case FSCameraProperty::BacklightCompensation:
        return ui->checkBoxLockBacklightCompensation;
    case FSCameraProperty::Gain:
        return ui->checkBoxLockGain;
    case FSCameraProperty::PowerlineFrequency:
        return ui->checkBoxLockPowerlineFrequency;
    case FSCameraProperty::Pan:
        return ui->checkBoxLockPan;
    case FSCameraProperty::Tilt:
        return ui->checkBoxLockTilt;
    case FSCameraProperty::Roll:
        return ui->checkBoxLockRoll;
    case FSCameraProperty::Zoom:
        return ui->checkBoxLockZoom;
    case FSCameraProperty::Exposure:
        return ui->checkBoxLockExposure;
    case FSCameraProperty::Iris:
        return ui->checkBoxLockIris;
    case FSCameraProperty::Focus:
        return ui->checkBoxLockFocus;
    }

    return nullptr;
}

int FSCameraSettingsDialogPrivate::valueForSlider(int value, const FSRangeParams &rangeParams)
{
    if (!rangeParams.isNull() && rangeParams.stepValue() != 0) {
        const int resultValue = value / rangeParams.stepValue();

        if (resultValue > rangeParams.maxValue() / rangeParams.stepValue())
            return rangeParams.maxValue() / rangeParams.stepValue();
        else if (resultValue < rangeParams.minValue() / rangeParams.stepValue())
            return rangeParams.minValue() / rangeParams.stepValue();

        return resultValue;
    }
    else {
        return value;
    }
}

int FSCameraSettingsDialogPrivate::valueForSpinBox(int value, const FSRangeParams &rangeParams)
{
    if (!rangeParams.isNull()) {
        int resultValue = qRound(double(value) / rangeParams.stepValue()) * rangeParams.stepValue();

        if (resultValue > rangeParams.maxValue())
            return rangeParams.maxValue();
        else if (resultValue < rangeParams.minValue())
            return rangeParams.minValue();

        return resultValue;
    }
    else {
        return value;
    }
}

int FSCameraSettingsDialogPrivate::valueFromSlider(int value, const FSRangeParams &rangeParams)
{
    if (!rangeParams.isNull()) {
        int resultValue = value * rangeParams.stepValue();

        if (resultValue > rangeParams.maxValue())
            return rangeParams.maxValue();
        else if (resultValue < rangeParams.minValue())
            return rangeParams.minValue();

        return resultValue;
    }
    else {
        return value;
    }
}

int FSCameraSettingsDialogPrivate::valueFromSpinBox(int value, const FSRangeParams &rangeParams)
{
    if (!rangeParams.isNull()) {
        int resultValue = qRound(double(value) / rangeParams.stepValue()) * rangeParams.stepValue();

        if (resultValue > rangeParams.maxValue())
            return rangeParams.maxValue();
        else if (resultValue < rangeParams.minValue())
            return rangeParams.minValue();

        return resultValue;
    }
    else {
        return value;
    }
}

void FSCameraSettingsDialogPrivate::setEditorsRange(Ui::FSCameraSettingsDialog *ui,
                                                    FSCameraProperty property,
                                                    const FSRangeParams &rangeParams) const
{
    if (!camera)
        return;

    bool isLock = (lockPropertiesManager &&
                   lockPropertiesManager->isLockedProperty(camera->devicePath(), property));

    switch (property) {
    case FSCameraProperty::None:
        return;
    case FSCameraProperty::Brightness:
    case FSCameraProperty::Contrast:
    case FSCameraProperty::Hue:
    case FSCameraProperty::Saturation:
    case FSCameraProperty::Sharpness:
    case FSCameraProperty::Gamma:
    case FSCameraProperty::WhiteBalance:
    case FSCameraProperty::BacklightCompensation:
    case FSCameraProperty::Gain:
    case FSCameraProperty::Pan:
    case FSCameraProperty::Tilt:
    case FSCameraProperty::Roll:
    case FSCameraProperty::Zoom:
    case FSCameraProperty::Exposure:
    case FSCameraProperty::Iris:
    case FSCameraProperty::Focus:
        setLabelRangeParams(getLabel(ui, property), rangeParams);
        setSliderRangeParams(getSlider(ui, property), rangeParams, isLock);
        setSpinBoxRangeParams(getSpinBox(ui, property), rangeParams, isLock);
        setAutoCheckBoxRangeParams(getAutoCheckBox(ui, property), rangeParams, isLock);
        setLockCheckBoxRangeParams(getLockCheckBox(ui, property), rangeParams);
        break;
    case FSCameraProperty::ColorEnable:
        setLabelRangeParams(getLabel(ui, property), rangeParams);
        setCheckBoxColorEnableRangeParams(ui, rangeParams, isLock);
        setLockCheckBoxRangeParams(getLockCheckBox(ui, property), rangeParams);
        break;
    case FSCameraProperty::PowerlineFrequency:
        setLabelRangeParams(getLabel(ui, property), rangeParams);
        setComboBoxPowerlineFrequencyRangeParams(ui, rangeParams, isLock);
        setLockCheckBoxRangeParams(getLockCheckBox(ui, property), rangeParams);
        break;
    }
}

void FSCameraSettingsDialogPrivate::setLabelRangeParams(QLabel *label,
                                                        const FSRangeParams &rangeParams)
{
    if (label) {
        label->setEnabled(!rangeParams.isNull());
    }
}

void FSCameraSettingsDialogPrivate::setSliderRangeParams(QSlider *slider,
                                                         const FSRangeParams &rangeParams,
                                                         bool isLock)
{
    if (slider) {
        slider->setEnabled(!isLock && !rangeParams.isNull());

        slider->blockSignals(true);

        if (!rangeParams.isNull()) {
            slider->setMinimum(valueForSlider(rangeParams.minValue(), rangeParams));
            slider->setMaximum(valueForSlider(rangeParams.maxValue(), rangeParams));
            slider->setSingleStep(1);
            slider->setPageStep(1);
            slider->setValue(valueForSlider(rangeParams.value(), rangeParams));
        } else {
            slider->setMinimum(0);
            slider->setMaximum(1);
            slider->setSingleStep(1);
            slider->setValue(0);
        }

        slider->blockSignals(false);
    }
}

void FSCameraSettingsDialogPrivate::setSpinBoxRangeParams(QSpinBox *spinBox,
                                                          const FSRangeParams &rangeParams,
                                                          bool isLock)
{
    if (spinBox) {
        spinBox->setEnabled(!isLock && !rangeParams.isNull());

        spinBox->blockSignals(true);

        if (!rangeParams.isNull()) {
            spinBox->setMinimum(valueForSpinBox(rangeParams.minValue(), rangeParams));
            spinBox->setMaximum(valueForSpinBox(rangeParams.maxValue(), rangeParams));
            spinBox->setSingleStep(rangeParams.stepValue());
            spinBox->setValue(valueForSpinBox(rangeParams.value(), rangeParams));
        } else {
            spinBox->setMinimum(0);
            spinBox->setMaximum(1);
            spinBox->setSingleStep(1);
            spinBox->setValue(0);
        }

        spinBox->blockSignals(false);
    }
}

void FSCameraSettingsDialogPrivate::setAutoCheckBoxRangeParams(QCheckBox *autoCheckBox,
                                                               const FSRangeParams &rangeParams,
                                                               bool isLock)
{
    if (autoCheckBox) {
        autoCheckBox->setEnabled(!isLock && !rangeParams.isNull() && rangeParams.isSupportAutoControl());
    }
}

void FSCameraSettingsDialogPrivate::setLockCheckBoxRangeParams(QCheckBox *lockCheckBox,
                                                               const FSRangeParams &rangeParams)
{
    if (lockCheckBox) {
        lockCheckBox->setEnabled(!rangeParams.isNull());
    }
}

void FSCameraSettingsDialogPrivate::setCheckBoxColorEnableRangeParams(Ui::FSCameraSettingsDialog *ui,
                                                                      const FSRangeParams &rangeParams,
                                                                      bool isLock)
{
    ui->checkBoxColorEnable->setEnabled(!isLock && !rangeParams.isNull());
}

void FSCameraSettingsDialogPrivate::setComboBoxPowerlineFrequencyRangeParams(Ui::FSCameraSettingsDialog *ui,
                                                                             const FSRangeParams &rangeParams,
                                                                             bool isLock)
{
    const int oldCurrentIndex = ui->comboBoxPowerlineFrequency->currentIndex();
    const QString oldCurrentText = ui->comboBoxPowerlineFrequency->itemText(oldCurrentIndex);
    const int oldItemsCount = ui->comboBoxPowerlineFrequency->count();

    ui->comboBoxPowerlineFrequency->setEnabled(!isLock && !rangeParams.isNull());

    ui->comboBoxPowerlineFrequency->blockSignals(true);
    ui->comboBoxPowerlineFrequency->clear();

    if (!rangeParams.isNull()) {
        const int delta = rangeParams.maxValue() - rangeParams.minValue();

        if (delta <= 3 && delta > 0) {
            for (int i = rangeParams.minValue(); i <= rangeParams.maxValue(); i += rangeParams.stepValue()) {
                switch (i) {
                case 1:
                    ui->comboBoxPowerlineFrequency->addItem(STRING_POWERLINE_FREQUENCY_50);
                    break;
                case 2:
                    ui->comboBoxPowerlineFrequency->addItem(STRING_POWERLINE_FREQUENCY_60);
                    break;
                default:
                    continue;
                }
            }
        }
    }

    if (ui->comboBoxPowerlineFrequency->count() != 0) {
        if (oldItemsCount == ui->comboBoxPowerlineFrequency->count()) {
            ui->comboBoxPowerlineFrequency->setCurrentIndex(oldCurrentIndex);
        } else {
            int newCurrentIndex = ui->comboBoxPowerlineFrequency->findText(oldCurrentText);

            if (newCurrentIndex != -1) {
                ui->comboBoxPowerlineFrequency->setCurrentIndex(newCurrentIndex);
            } else {
                ui->comboBoxPowerlineFrequency->setCurrentIndex(0);
            }
        }
    }

    ui->comboBoxPowerlineFrequency->blockSignals(false);
}

void FSCameraSettingsDialogPrivate::setEditorsValue(Ui::FSCameraSettingsDialog *ui,
                                                    FSCameraProperty property,
                                                    const FSValueParams &valueParams) const
{
    if (!camera)
        return;

    bool isLock = false;

    if (lockPropertiesManager && lockPropertiesManager->isLockedProperty(camera->devicePath(), property))
        isLock = true;

    FSRangeParams rangeParams;

    switch (property) {
    case FSCameraProperty::None:
        return;
    case FSCameraProperty::Brightness:
    case FSCameraProperty::Contrast:
    case FSCameraProperty::Hue:
    case FSCameraProperty::Saturation:
    case FSCameraProperty::Sharpness:
    case FSCameraProperty::Gamma:
    case FSCameraProperty::WhiteBalance:
    case FSCameraProperty::BacklightCompensation:
    case FSCameraProperty::Gain:
    case FSCameraProperty::Pan:
    case FSCameraProperty::Tilt:
    case FSCameraProperty::Roll:
    case FSCameraProperty::Zoom:
    case FSCameraProperty::Exposure:
    case FSCameraProperty::Iris:
    case FSCameraProperty::Focus:
        rangeParams = getRangeParams(property);
        setLabelSelection(getLabel(ui, property), valueParams, getDefaultValue(property), isLock);
        setSliderValueParams(getSlider(ui, property), valueParams, rangeParams, isLock);
        setSpinBoxValueParams(getSpinBox(ui, property), valueParams, rangeParams, isLock);
        setAutoCheckBoxValueParams(getAutoCheckBox(ui, property), valueParams, rangeParams, isLock);
        break;
    case FSCameraProperty::ColorEnable:
        setLabelSelection(getLabel(ui, property), valueParams, getDefaultValue(property), isLock);
        setCheckBoxColorEnableValueParams(ui, valueParams, isLock);
        break;
    case FSCameraProperty::PowerlineFrequency:
        setLabelSelection(getLabel(ui, property), valueParams, getDefaultValue(property), isLock);
        setComboBoxPowerlineFrequencyValueParams(ui, valueParams, isLock);
        break;
    }
}

void FSCameraSettingsDialogPrivate::setLabelSelection(QLabel *label,
                                                      const FSValueParams &valueParams,
                                                      const FSValueParams &defaultParams,
                                                      bool isLock)
{
    if (label) {
        QFont font = label->font();

        bool isEqualParams = false;

        if (!valueParams.isNull() && !defaultParams.isNull()) {
            if (valueParams.isManualControl() == defaultParams.isManualControl()) {
                isEqualParams = (valueParams.value() == defaultParams.value());
            } else if (valueParams.isAutoControl() && defaultParams.isAutoControl()) {
                isEqualParams = true;
            }
        }

        font.setBold(!isEqualParams);
        font.setItalic(isLock);
        label->setFont(font);
    }
}

void FSCameraSettingsDialogPrivate::setSliderValueParams(QSlider *slider,
                                                         const FSValueParams &valueParams,
                                                         const FSRangeParams &rangeParams,
                                                         bool isLock)
{
    if (slider) {
        slider->blockSignals(true);

        if (!valueParams.isNull()) {
            slider->setEnabled(!isLock && !valueParams.isAutoControl());
            slider->setValue(valueForSlider(valueParams.value(), rangeParams));
        } else {
            slider->setEnabled(false);
            slider->setValue(0);
        }

        slider->blockSignals(false);
    }
}

void FSCameraSettingsDialogPrivate::setSpinBoxValueParams(QSpinBox *spinBox,
                                                          const FSValueParams &valueParams,
                                                          const FSRangeParams &rangeParams,
                                                          bool isLock)
{
    if (spinBox && (spinBox != spinBox->focusWidget())) {
        spinBox->blockSignals(true);

        if (!valueParams.isNull()) {
            spinBox->setEnabled(!isLock && !valueParams.isAutoControl());
            spinBox->setValue(valueForSpinBox(valueParams.value(), rangeParams));
        } else {
            spinBox->setEnabled(false);
            spinBox->setValue(0);
        }

        spinBox->blockSignals(false);
    }
}

void FSCameraSettingsDialogPrivate::setAutoCheckBoxValueParams(QCheckBox *autoCheckBox,
                                                               const FSValueParams &valueParams,
                                                               const FSRangeParams &rangeParams,
                                                               bool isLock)
{
    if (autoCheckBox) {
        autoCheckBox->blockSignals(true);

        if (!valueParams.isNull()) {
            autoCheckBox->setEnabled(!isLock && !rangeParams.isNull() && rangeParams.isSupportAutoControl());
            autoCheckBox->setChecked(valueParams.isAutoControl());
        } else {
            autoCheckBox->setEnabled(false);
            autoCheckBox->setChecked(false);
        }

        autoCheckBox->blockSignals(false);
    }
}

void FSCameraSettingsDialogPrivate::setCheckBoxColorEnableValueParams(Ui::FSCameraSettingsDialog *ui,
                                                                      const FSValueParams &valueParams,
                                                                      bool isLock)
{
    ui->checkBoxColorEnable->blockSignals(true);

    if (!valueParams.isNull()) {
        ui->checkBoxColorEnable->setEnabled(!isLock);
        ui->checkBoxColorEnable->setChecked(valueParams.value());
    } else {
        ui->checkBoxColorEnable->setEnabled(false);
        ui->checkBoxColorEnable->setChecked(false);
    }

    ui->checkBoxColorEnable->blockSignals(false);
}

void FSCameraSettingsDialogPrivate::setComboBoxPowerlineFrequencyValueParams(Ui::FSCameraSettingsDialog *ui,
                                                                             const FSValueParams &valueParams,
                                                                             bool isLock)
{
    ui->comboBoxPowerlineFrequency->blockSignals(true);

    if (!valueParams.isNull()) {
        ui->comboBoxPowerlineFrequency->setEnabled(!isLock);
        switch (valueParams.value()) {
        case 1:
            ui->comboBoxPowerlineFrequency->setCurrentIndex(ui->comboBoxPowerlineFrequency->findText(STRING_POWERLINE_FREQUENCY_50));
            break;
        case 2:
            ui->comboBoxPowerlineFrequency->setCurrentIndex(ui->comboBoxPowerlineFrequency->findText(STRING_POWERLINE_FREQUENCY_60));
            break;
        default:
            ui->comboBoxPowerlineFrequency->setCurrentIndex(-1);
            break;
        }
    } else {
        ui->comboBoxPowerlineFrequency->setEnabled(false);
        ui->comboBoxPowerlineFrequency->setCurrentIndex(-1);
    }

    ui->comboBoxPowerlineFrequency->blockSignals(false);
}

bool FSCameraSettingsDialogPrivate::isEditorValueEnable(FSCameraProperty property) const
{
    return (umapCameraPropertyRangeParams.find(property) != umapCameraPropertyRangeParams.end());
}

FSRangeParams FSCameraSettingsDialogPrivate::getRangeParams(FSCameraProperty property) const
{
    FSCameraPropertyRangesUMap::const_iterator iterator = umapCameraPropertyRangeParams.find(property);
    if (iterator != umapCameraPropertyRangeParams.end()) {
        return iterator->second;
    }

    return FSRangeParams();
}

FSValueParams FSCameraSettingsDialogPrivate::getActualValue(FSCameraProperty property) const
{
    FSValueParams result;

    if (mode == FSCameraSettingsDialog::LockPropertiesMode) {
        if (!camera->get(property, result)) {
            qCritical("%s: Failed to get values for property \"%s\", devicePath=\"%s\"!",
                      FSCameraSettingsDialog::staticMetaObject.className(),
                      fsGetEnumName(property).toLocal8Bit().constData(),
                      camera->devicePath().toLocal8Bit().constData());
        }
    } else if (mode == FSCameraSettingsDialog::ChangeDefaultValuesMode) {
        FSCamerasStorage *camerasStorage = qobject_cast<FSCamerasStorage *>(camera->camerasStorage());
        if (camerasStorage && camerasStorage->isUserDefaultValueUsed(camera->devicePath(), property)) {
            result = camerasStorage->getUserDefaultValue(camera->devicePath(), property);
        } else {
            result = getDefaultValueFromRange(property);
        }
    } else {
        FSCamerasStorage *camerasStorage = qobject_cast<FSCamerasStorage *>(camera->camerasStorage());
        if (camerasStorage && camerasStorage->isCameraUserPresetValueUsed(camera->devicePath(), currentPresetName, property)) {
            result = camerasStorage->getCameraUserPresetValue(camera->devicePath(), currentPresetName, property);
        } else {
            result = getDefaultValue(property);
        }
    }

    return result;
}

FSValueParams FSCameraSettingsDialogPrivate::getDefaultValue(FSCameraProperty property) const
{
    FSValueParams result;

    if (!camera)
        return result;

    if ( mode == FSCameraSettingsDialog::LockPropertiesMode &&
         lockPropertiesManager &&
         lockPropertiesManager->isLockedProperty(camera->devicePath(), property) ) {
        return result;
    }

    if (mode == FSCameraSettingsDialog::ChangeDefaultValuesMode) {
        result = getDefaultValueFromRange(property);
    } else {
        FSCamerasStorage *camerasStorage = qobject_cast<FSCamerasStorage *>(camera->camerasStorage());
        if (camerasStorage && camerasStorage->isUserDefaultValueUsed(camera->devicePath(), property)) {
            result = camerasStorage->getUserDefaultValue(camera->devicePath(), property);
        } else {
            result = getDefaultValueFromRange(property);
        }
    }

    return result;
}

FSValueParams FSCameraSettingsDialogPrivate::getDefaultValueFromRange(FSCameraProperty property) const
{
    FSCameraPropertyRangesUMap::const_iterator iterator = umapCameraPropertyRangeParams.find(property);
    if (iterator != umapCameraPropertyRangeParams.end()) {
        const FSRangeParams &rangeParams = iterator->second;

        if (rangeParams.isSupportManualControl())
            return FSValueParams(rangeParams.value(), FSValueParams::genManualControlFlags());
        else if (rangeParams.isSupportAutoControl())
            return FSValueParams(rangeParams.value(), FSValueParams::genAutoControlFlags());
        else
            return FSValueParams(rangeParams.value());
    }

    return FSValueParams();
}

FSValueParams FSCameraSettingsDialogPrivate::getEditorValue(Ui::FSCameraSettingsDialog *ui,
                                                            FSCameraProperty property,
                                                            EditorType editorType) const
{
    FSValueParams lockValueParams;

    if (mode == FSCameraSettingsDialog::LockPropertiesMode && lockPropertiesManager && camera) {
        lockValueParams = lockPropertiesManager->lockedValueParams(camera->devicePath(), property);
    }

    if (!lockValueParams.isNull())
        return lockValueParams;

    switch (property) {
    case FSCameraProperty::None:
        return FSValueParams();
    case FSCameraProperty::Brightness:
    case FSCameraProperty::Contrast:
    case FSCameraProperty::Hue:
    case FSCameraProperty::Saturation:
    case FSCameraProperty::Sharpness:
    case FSCameraProperty::Gamma:
    case FSCameraProperty::WhiteBalance:
    case FSCameraProperty::BacklightCompensation:
    case FSCameraProperty::Gain:
    case FSCameraProperty::Pan:
    case FSCameraProperty::Tilt:
    case FSCameraProperty::Roll:
    case FSCameraProperty::Zoom:
    case FSCameraProperty::Exposure:
    case FSCameraProperty::Iris:
    case FSCameraProperty::Focus:
        return standartValueParams(ui, editorType, property);
    case FSCameraProperty::ColorEnable:
        return colorEnableValueParams(ui);
    case FSCameraProperty::PowerlineFrequency:
        return powerlineFrequencyValueParams(ui);
    }

    return FSValueParams();
}

FSValueParams FSCameraSettingsDialogPrivate::standartValueParams(Ui::FSCameraSettingsDialog *ui,
                                                                 EditorType editorType,
                                                                 FSCameraProperty property) const
{
    QSlider *slider = getSlider(ui, property);
    QSpinBox *spinBox = getSpinBox(ui, property);

    if (!slider || !spinBox)
        return FSValueParams();

    int value = 0;

    switch (editorType) {
    case NoneEditorType:
    case SliderEditorType:
        value = valueFromSlider(slider->value(), getRangeParams(property));
        break;
    case SpinBoxEditorType:
    case AutoCheckBoxEditorType:
        value = valueFromSpinBox(spinBox->value(), getRangeParams(property));
        break;
    }

    QCheckBox *autoCheckBox = getAutoCheckBox(ui, property);

    if (!autoCheckBox)
        return FSValueParams();

    return FSValueParams(value, autoCheckBox->isChecked() ? FSValueParams::genAutoControlFlags() : FSValueParams::genManualControlFlags());
}

FSValueParams FSCameraSettingsDialogPrivate::colorEnableValueParams(Ui::FSCameraSettingsDialog *ui) const
{
    return FSValueParams(ui->checkBoxColorEnable->isChecked() ? 1 : 0, FSValueParams::genManualControlFlags());
}

FSValueParams FSCameraSettingsDialogPrivate::powerlineFrequencyValueParams(Ui::FSCameraSettingsDialog *ui) const
{
    QComboBox *valueComboBox = ui->comboBoxPowerlineFrequency;

    if (valueComboBox->count() == 0)
        return FSValueParams();

    long currentValue = 1;

    if (valueComboBox->currentIndex() == valueComboBox->findText(STRING_POWERLINE_FREQUENCY_60))
        currentValue = 2;

    return FSValueParams(currentValue);
}

void FSCameraSettingsDialogPrivate::connectingEditors(Ui::FSCameraSettingsDialog *ui,
                                                      FSCameraProperty property)
{
    switch (property) {
    case FSCameraProperty::None:
        return;
    case FSCameraProperty::Brightness:
    case FSCameraProperty::Contrast:
    case FSCameraProperty::Hue:
    case FSCameraProperty::Saturation:
    case FSCameraProperty::Sharpness:
    case FSCameraProperty::Gamma:
    case FSCameraProperty::WhiteBalance:
    case FSCameraProperty::BacklightCompensation:
    case FSCameraProperty::Gain:
    case FSCameraProperty::Pan:
    case FSCameraProperty::Tilt:
    case FSCameraProperty::Roll:
    case FSCameraProperty::Zoom:
    case FSCameraProperty::Exposure:
    case FSCameraProperty::Iris:
    case FSCameraProperty::Focus:
        connectingStandartEditors(ui, property);
        break;
    case FSCameraProperty::ColorEnable:
        connectingColorEnableEditor(ui);
        break;
    case FSCameraProperty::PowerlineFrequency:
        connectingPowerlineFrequencyEditor(ui);
        break;
    }
}

void FSCameraSettingsDialogPrivate::connectingStandartEditors(Ui::FSCameraSettingsDialog *ui,
                                                              FSCameraProperty property)
{
    QSlider *slider = getSlider(ui, property);
    if (slider) {
        propertyChangeSignalMapper->setMapping(slider, genEditorTypeProperty(SliderEditorType, property));

        QObject::connect(slider,                   SIGNAL(valueChanged(int)),
                         propertyChangeSignalMapper, SLOT(map()));
    }

    QSpinBox *spinBox = getSpinBox(ui, property);
    if (spinBox) {
        propertyChangeSignalMapper->setMapping(spinBox, genEditorTypeProperty(SpinBoxEditorType, property));

        QObject::connect(spinBox,                  SIGNAL(valueChanged(int)),
                         propertyChangeSignalMapper, SLOT(map()));
    }

    QCheckBox *checkBox = getAutoCheckBox(ui, property);
    if (checkBox) {
        propertyChangeSignalMapper->setMapping(checkBox, genEditorTypeProperty(AutoCheckBoxEditorType, property));

        QObject::connect(checkBox,                 SIGNAL(stateChanged(int)),
                         propertyChangeSignalMapper, SLOT(map()));
    }
}

void FSCameraSettingsDialogPrivate::connectingColorEnableEditor(Ui::FSCameraSettingsDialog *ui)
{
    propertyChangeSignalMapper->setMapping(ui->checkBoxColorEnable, genEditorTypeProperty(NoneEditorType, FSCameraProperty::ColorEnable));

    QObject::connect(ui->checkBoxColorEnable,  SIGNAL(stateChanged(int)),
                     propertyChangeSignalMapper, SLOT(map()));
}

void FSCameraSettingsDialogPrivate::connectingPowerlineFrequencyEditor(Ui::FSCameraSettingsDialog *ui)
{
    propertyChangeSignalMapper->setMapping(ui->comboBoxPowerlineFrequency, genEditorTypeProperty(NoneEditorType, FSCameraProperty::PowerlineFrequency));

    QObject::connect(ui->comboBoxPowerlineFrequency, SIGNAL(currentIndexChanged(int)),
                     propertyChangeSignalMapper,       SLOT(map()));
}

void FSCameraSettingsDialogPrivate::connectingLockCheckBox(Ui::FSCameraSettingsDialog *ui,
                                                           FSCameraProperty property)
{
    QCheckBox *checkBox = getLockCheckBox(ui, property);
    if (checkBox) {
        lockSignalMapper->setMapping(checkBox, property);

        QObject::connect(checkBox,       SIGNAL(stateChanged(int)),
                         lockSignalMapper, SLOT(map()));
    }
}

void FSCameraSettingsDialogPrivate::readRangesFromDevice(Ui::FSCameraSettingsDialog *ui, FSCameraProperty property)
{
    FSRangeParams rangeParams;
    camera->getRange(property, rangeParams);

    if (!rangeParams.isNull()) {
        umapCameraPropertyRangeParams.insert_or_assign(property, rangeParams);
    } else {
        FSCameraPropertyRangesUMap::iterator iterator = umapCameraPropertyRangeParams.find(property);
        if (iterator != umapCameraPropertyRangeParams.end()) {
            umapCameraPropertyRangeParams.erase(iterator);
        }
    }

    setEditorsRange(ui, property, rangeParams);
}

void FSCameraSettingsDialogPrivate::updateValue(Ui::FSCameraSettingsDialog *ui, FSCameraProperty property)
{
    FSCameraSettingsDialogPrivate::setEditorsValue(ui, property, getActualValue(property));
}

int FSCameraSettingsDialogPrivate::genEditorTypeProperty(EditorType editorType, FSCameraProperty property)
{
    return ((editorType * MAX_FS_CAMERA_PROPERTY_COUNT) + property);
}

FSCameraSettingsDialogPrivate::EditorType FSCameraSettingsDialogPrivate::getEditorType(int editorTypeProperty)
{
    return FSCameraSettingsDialogPrivate::EditorType(int(editorTypeProperty / MAX_FS_CAMERA_PROPERTY_COUNT));
}

FSCameraProperty FSCameraSettingsDialogPrivate::getProperty(int editorTypeProperty)
{
    return FSCameraProperty(int(editorTypeProperty % MAX_FS_CAMERA_PROPERTY_COUNT));
}

void FSCameraSettingsDialogPrivate::sendValueParams(Ui::FSCameraSettingsDialog *ui,
                                                    FSCameraProperty property,
                                                    EditorType editorType)
{
    FSValueParams valueParams = getEditorValue(ui, property, editorType);

    if (valueParams.isNull())
        return;

    if (camera->set(property, valueParams)) {
        updateValue(ui, property);
    } else {
        qCritical("%s: Failed to set values(value:\"%ld\", flags:\"%ld\") for property \"%s\", devicePath=\"%s\"!",
                  FSCameraSettingsDialog::staticMetaObject.className(),
                  valueParams.value(),
                  valueParams.flags(),
                  fsGetEnumName(property).toLocal8Bit().constData(),
                  camera->devicePath().toLocal8Bit().constData());
    }
}

FSCameraSettingsDialog::FSCameraSettingsDialog(FSCamera *camera, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FSCameraSettingsDialog)
{
    init(camera);
}

FSCameraSettingsDialog::~FSCameraSettingsDialog()
{
    delete d->valueUpdateTimer;
    delete d->propertyChangeSignalMapper;
    delete d->lockSignalMapper;

    delete d;
    d = nullptr;

    delete ui;
}

FSCamera *FSCameraSettingsDialog::camera() const
{
    return d->camera;
}

void FSCameraSettingsDialog::setMode(Mode mode)
{
    if (d->mode != mode) {
        d->mode = mode;

        updateModeParams();
        updateWidgetsByMode();
    }
}

FSCameraSettingsDialog::Mode FSCameraSettingsDialog::mode() const
{
    return d->mode;
}

void FSCameraSettingsDialog::setLockPropertiesManager(FSLockPropertiesManager *lockPropertiesManager)
{
    if (d->lockPropertiesManager != lockPropertiesManager) {
        if (d->lockPropertiesManager) {
            disconnect(d->lockPropertiesManager, SIGNAL(lockedProperty(DevicePath,FSCameraProperty)),
                       this,                       SLOT(lockedProperty(DevicePath,FSCameraProperty)));
            disconnect(d->lockPropertiesManager, SIGNAL(unlockedProperty(DevicePath,FSCameraProperty)),
                       this,                       SLOT(unlockedProperty(DevicePath,FSCameraProperty)));

            disconnect(d->lockPropertiesManager, SIGNAL(switchedToManualMode(DevicePath)),
                       this,                       SLOT(updateCurrentLockPresetIndex(DevicePath)));
            disconnect(d->lockPropertiesManager, SIGNAL(lockedPreset(DevicePath,QString)),
                       this,                       SLOT(updateCurrentLockPresetIndex(DevicePath)));
            disconnect(d->lockPropertiesManager, SIGNAL(unlockedPreset(DevicePath)),
                       this,                       SLOT(updateCurrentLockPresetIndex(DevicePath)));
        }

        d->lockPropertiesManager = lockPropertiesManager;

        if (d->lockPropertiesManager) {
            connect(d->lockPropertiesManager, SIGNAL(lockedProperty(DevicePath,FSCameraProperty)),
                    this,                       SLOT(lockedProperty(DevicePath,FSCameraProperty)));
            connect(d->lockPropertiesManager, SIGNAL(unlockedProperty(DevicePath,FSCameraProperty)),
                    this,                       SLOT(unlockedProperty(DevicePath,FSCameraProperty)));

            connect(d->lockPropertiesManager, SIGNAL(switchedToManualMode(DevicePath)),
                    this,                       SLOT(updateCurrentLockPresetIndex(DevicePath)));
            connect(d->lockPropertiesManager, SIGNAL(lockedPreset(DevicePath,QString)),
                    this,                       SLOT(updateCurrentLockPresetIndex(DevicePath)));
            connect(d->lockPropertiesManager, SIGNAL(unlockedPreset(DevicePath)),
                    this,                       SLOT(updateCurrentLockPresetIndex(DevicePath)));
        }

        updateModeParams();
        updateWidgetsByMode();
    }
}

FSLockPropertiesManager *FSCameraSettingsDialog::lockPropertiesManager() const
{
    return d->lockPropertiesManager;
}

void FSCameraSettingsDialog::setCameraValueUpdateEnable(bool isEnable)
{
    if (isEnable)
        d->valueUpdateTimer->start();
    else
        d->valueUpdateTimer->stop();
}

bool FSCameraSettingsDialog::isCameraValueUpdateEnable() const
{
    return d->valueUpdateTimer->isActive();
}

void FSCameraSettingsDialog::setCameraValueUpdateInterval(int msec)
{
    d->valueUpdateTimer->setInterval(msec);
}

int FSCameraSettingsDialog::cameraValueUpdateInterval() const
{
    return d->valueUpdateTimer->interval();
}

void FSCameraSettingsDialog::setCameraValueSendEnable(bool isEnable)
{
    if (d->isCameraValueSendEnable != isEnable) {
        d->isCameraValueSendEnable = isEnable;
    }
}

bool FSCameraSettingsDialog::isCameraValueSendEnable() const
{
    return d->isCameraValueSendEnable;
}

void FSCameraSettingsDialog::updateWindowTitle()
{
    setWindowTitle(d->origWindowTitle + " \"" + d->camera->displayName() + "\"");
}

void FSCameraSettingsDialog::updateValueParams(FSCameraProperty property)
{
    d->updateValue(ui, property);
}

void FSCameraSettingsDialog::setValueParams(FSCameraProperty property,
                                            const FSValueParams &valueParams)
{
    d->setEditorsValue(ui, property, valueParams);
}

bool FSCameraSettingsDialog::isValueEnable(FSCameraProperty property) const
{
    return d->isEditorValueEnable(property);
}

FSValueParams FSCameraSettingsDialog::valueParams(FSCameraProperty property) const
{
    if (d->isEditorValueEnable(property))
        return d->getEditorValue(ui, property);

    return FSValueParams();
}

void FSCameraSettingsDialog::sendValuesToCameraStorage()
{
    FSCamerasStorage *camerasStorage = qobject_cast<FSCamerasStorage *>(d->camera->camerasStorage());

    if (!camerasStorage || d->mode == FSCameraSettingsDialog::LockPropertiesMode)
        return;

    const QString &devicePath = d->camera->devicePath();

    if (d->mode == FSCameraSettingsDialog::ChangeDefaultValuesMode) {
        camerasStorage->userDefaultValuesRemove(devicePath);

        for (const FSCameraProperty &property : fsAllCameraProperties()) {
            const FSValueParams currentValue = this->valueParams(property);

            if (!currentValue.isNull()) {
                const FSValueParams defaultValue = d->getDefaultValueFromRange(property);

                if (!defaultValue.isNull() && currentValue != defaultValue) {
                    camerasStorage->setUserDefaultValue(devicePath,
                                                        property,
                                                        currentValue);
                }
            }
        }
    } else {
        camerasStorage->cameraUserPresetsRemove(devicePath);

        saveCurrentPresetValues();

        FSCameraUserPresetsUMap umapCameraUserPresets;

        for (const auto &[presetName, propertyValues] : d->umapCameraUserPresets) {
            if (!presetName.isEmpty() && !propertyValues.empty()) {
                umapCameraUserPresets.insert( { presetName, propertyValues } );
            }
        }

        camerasStorage->setCameraUserPresets(devicePath, umapCameraUserPresets);
    }
}

void FSCameraSettingsDialog::done(int r)
{
    if (r == QDialog::Accepted) {
        if (d->mode == FSCameraSettingsDialog::ChangePresetMode) {
            saveCurrentPresetValues();

            bool isContaintsEmptyPresets = false;

            for (const auto &[presetName, propertyValues] : d->umapCameraUserPresets) {
                if (!presetName.isEmpty() && propertyValues.empty()) {
                    isContaintsEmptyPresets = true;
                    break;
                }
            }

            if (isContaintsEmptyPresets) {
                QMessageBox msgBox(QMessageBox::Question,
                                   tr("Empty presets"),
                                   tr("Empty presets found and will be deleted. Continue and delete empty presets?"),
                                   QMessageBox::Yes | QMessageBox::Cancel,
                                   this);

                QString emptyPresetText;

                for (const auto &[presetName, propertyValues] : d->umapCameraUserPresets) {
                    if (!presetName.isEmpty() && propertyValues.empty()) {
                        emptyPresetText += QString("\n\"%1\"").arg(presetName);
                    }
                }

                msgBox.setDetailedText(tr("Empty preset list: %1").arg(emptyPresetText));

                if (msgBox.exec() == QMessageBox::Cancel)
                    return;
            }
        }
    }

    QDialog::done(r);
}

void FSCameraSettingsDialog::keyPressEvent(QKeyEvent *event)
{
    if ( event->key() == Qt::Key_Enter ||
         event->key() == Qt::Key_Return ) {
        QWidget *nextFocusWidget = focusWidget();

        while ( nextFocusWidget &&
                nextFocusWidget->nextInFocusChain() ) {
            nextFocusWidget = nextFocusWidget->nextInFocusChain();

            if (nextFocusWidget->isEnabled())
                break;
        }

        if (nextFocusWidget)
            nextFocusWidget->setFocus();
        else
            focusWidget()->clearFocus();
        return;
    }

    QDialog::keyPressEvent(event);
}

bool FSCameraSettingsDialog::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::FocusIn || event->type() == QEvent::FocusOut) {
        QSpinBox *spinBox = qobject_cast<QSpinBox *>(object);

        if (spinBox) {
            if (event->type() == QEvent::FocusIn) {
                disconnect(spinBox,                     SIGNAL(valueChanged(int)),
                           d->propertyChangeSignalMapper, SLOT(map()));
            } else {
                connect(spinBox,                     SIGNAL(valueChanged(int)),
                        d->propertyChangeSignalMapper, SLOT(map()));
                emit spinBox->valueChanged(spinBox->value());
            }
        }
    }

    return QDialog::eventFilter(object, event);
}

void FSCameraSettingsDialog::init(FSCamera *camera)
{
    d = new FSCameraSettingsDialogPrivate();

    d->camera = camera;

    ui->setupUi(this);
    ui->tabWidget->setCurrentWidget(ui->tabVideoProcAmp);

    connect(fsTH, &FSTranslationsHelper::currentLanguageChanged,
            this, &FSCameraSettingsDialog::retranslate);

    connect(ui->comboBoxPresets, SIGNAL(currentIndexChanged(int)),
            this,                  SLOT(updateCurrentPresetRemoveButton()));
    connect(ui->comboBoxPresets, SIGNAL(currentIndexChanged(int)),
            this,                  SLOT(restorePresetValues(int)));

    connect(ui->toolButtonRenameCurrentPreset, SIGNAL(clicked()),
            this,                                SLOT(execRenameCurrentPresetName()));

    connect(ui->toolButtonRemoveCurrentPreset, SIGNAL(clicked()),
            this,                                SLOT(removeCurrentPreset()));

    updateMinimumWidthSpinBox();

    retranslatePropertyNames();

    readRangesFromDevice();

    d->valueUpdateTimer = new QTimer(this);
    d->valueUpdateTimer->setInterval(FSSettings::cameraValueUpdateInterval());
    connect(d->valueUpdateTimer, SIGNAL(timeout()),
            this,                  SLOT(updateValues()));

    d->origWindowTitle = windowTitle();
    updateWindowTitle();
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    initEditorsConnections();

    for (FSCameraProperty property: fsAllCameraProperties()) {
        QSpinBox *spinBox = d->getSpinBox(ui, property);

        if (spinBox)
            spinBox->installEventFilter(this);
    }

    updateModeParams();
    updateWidgetsByMode();
}

void FSCameraSettingsDialog::initEditorsConnections()
{
    d->propertyChangeSignalMapper = new QSignalMapper(this);

    connect(d->propertyChangeSignalMapper, SIGNAL(mappedInt(int)),
            this,                            SLOT(applyCurrentValueParams(int)),
            Qt::QueuedConnection);

    for (const FSCameraProperty &property : fsAllCameraProperties())
        d->connectingEditors(ui, property);

    d->lockSignalMapper = new QSignalMapper(this);

    connect(d->lockSignalMapper, SIGNAL(mappedInt(int)),
            this,                  SLOT(sendLockCameraProperty(int)),
            Qt::QueuedConnection);

    for (const FSCameraProperty &property : fsAllCameraProperties())
        d->connectingLockCheckBox(ui, property);
}

void FSCameraSettingsDialog::retranslatePropertyNames()
{
    for (const FSCameraProperty &property : fsAllCameraProperties()) {
        QLabel *label = FSCameraSettingsDialogPrivate::getLabel(ui, property);
        if (label) { label->setText(fsGetTrEnumName(property)); }
    }

    updateMinimumWidthLabels();
}

void FSCameraSettingsDialog::updateModeParams()
{
    switch (d->mode) {
    case FSCameraSettingsDialog::LockPropertiesMode:
        setCameraValueUpdateEnable(FSSettings::isCameraValueUpdateEnable());
        setCameraValueSendEnable(true);
        ui->buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::RestoreDefaults);
        break;
    case FSCameraSettingsDialog::ChangeDefaultValuesMode:
    case FSCameraSettingsDialog::ChangePresetMode:
        setCameraValueUpdateEnable(false);
        setCameraValueSendEnable(false);
        ui->buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults);
        break;
    }

    updatePresetsFromCameraStorage();

    QPushButton *restoreDefaultsButton = ui->buttonBox->button(QDialogButtonBox::RestoreDefaults);
    if (restoreDefaultsButton) {
        connect(restoreDefaultsButton, SIGNAL(clicked(bool)),
                this,                    SLOT(restoreDefaultValues()));
    }

    ui->widgetPresets->setVisible(d->mode != FSCameraSettingsDialog::ChangeDefaultValuesMode);
    ui->toolButtonRemoveCurrentPreset->setVisible(d->mode == FSCameraSettingsDialog::ChangePresetMode);
    ui->toolButtonRenameCurrentPreset->setVisible(d->mode == FSCameraSettingsDialog::ChangePresetMode);

    // Lock widgets
    {
        const bool isLockEnabled = (d->mode == FSCameraSettingsDialog::LockPropertiesMode &&
                                    d->lockPropertiesManager);

        ui->labelVideoProcAmpLock->setVisible(isLockEnabled);
        ui->labelCameraControlLock->setVisible(isLockEnabled);

        ui->lineVideoProcAmpAutoWidgets->setVisible(isLockEnabled);
        ui->lineCameraControlAutoWidgets->setVisible(isLockEnabled);

        ui->lineVideoProcAmpLockWidgets->setVisible(isLockEnabled);
        ui->lineCameraControlLockWidgets->setVisible(isLockEnabled);

        for (const FSCameraProperty &property : fsAllCameraProperties()) {
            QCheckBox *lockCheckBox = d->getLockCheckBox(ui, property);

            if (lockCheckBox) {
                lockCheckBox->setVisible(isLockEnabled);
            }
        }
    }
}

void FSCameraSettingsDialog::updateWidgetsByMode()
{
    if (!d->camera)
        return;

    for (const auto& [property, rangeParams] : d->umapCameraPropertyRangeParams) {
        d->setEditorsRange(ui, property, rangeParams);
    }

    updateValues();

    for (const FSCameraProperty &property : fsAllCameraProperties()) {
        QCheckBox *lockCheckBox = d->getLockCheckBox(ui, property);

        if (lockCheckBox) {
            lockCheckBox->blockSignals(true);
            lockCheckBox->setChecked(d->mode == FSCameraSettingsDialog::LockPropertiesMode &&
                                     d->lockPropertiesManager &&
                                     d->lockPropertiesManager->isLockedProperty(d->camera->devicePath(), property));
            lockCheckBox->blockSignals(false);
        }
    }
}

void FSCameraSettingsDialog::setLockProperty(const DevicePath &devicePath,
                                             FSCameraProperty property,
                                             bool isLock)
{
    if (!d->camera || devicePath != d->camera->devicePath())
        return;

    d->updateValue(ui, property);

    QCheckBox *checkBox = d->getLockCheckBox(ui, property);

    if (checkBox) {
        checkBox->blockSignals(true);
        checkBox->setChecked(isLock);
        checkBox->blockSignals(false);
    }
}

void FSCameraSettingsDialog::updateCurrentLockPresetIndex()
{
    if (d->mode == FSCameraSettingsDialog::LockPropertiesMode) {
        if (d->lockPropertiesManager) {
            const FSLockPropertiesManager::LockMode lockMode = d->lockPropertiesManager->currentLockMode(d->camera->devicePath());
            if (lockMode == FSLockPropertiesManager::ManualLockMode) {
                ui->comboBoxPresets->setCurrentIndex(ui->comboBoxPresets->findText(MANUAL_PRESET));
            } else if (lockMode == FSLockPropertiesManager::PresetLockMode) {
                ui->comboBoxPresets->setCurrentIndex(ui->comboBoxPresets->findText(d->lockPropertiesManager->lockPresetName(d->camera->devicePath())));
            } else {
                ui->comboBoxPresets->setCurrentIndex(-1);
            }
        } else {
            ui->comboBoxPresets->setCurrentIndex(0);
        }
    }
}

void FSCameraSettingsDialog::saveCurrentPresetValues()
{
    if (d->currentPresetName.isEmpty())
        return;

    FSCameraUserPresetsUMap::iterator iterator = d->umapCameraUserPresets.find(d->currentPresetName);

    if (iterator == d->umapCameraUserPresets.end()) {
        iterator = d->umapCameraUserPresets.insert( { d->currentPresetName, FSCameraPropertyValuesUMap() } ).first;
    }

    FSCameraPropertyValuesUMap &umapCameraPropertyValues = iterator->second;

    umapCameraPropertyValues.clear();

    for (const FSCameraProperty &property : fsAllCameraProperties()) {
        const FSValueParams currentValue = this->valueParams(property);

        if (!currentValue.isNull()) {
            const FSValueParams defaultValue = d->getDefaultValue(property);

            if (!defaultValue.isNull() && currentValue != defaultValue) {
                umapCameraPropertyValues.insert( { property, currentValue } );
            }
        }
    }
}

void FSCameraSettingsDialog::restorePresetValues(const QString &presetName)
{
    if (d->mode != FSCameraSettingsDialog::ChangePresetMode)
        return;

    restoreDefaultValues();

    FSCameraUserPresetsUMap::iterator iterator = d->umapCameraUserPresets.find(presetName);
    if (iterator != d->umapCameraUserPresets.end()) {
        for (const auto &[property, valueParams] : iterator->second) {
            d->setEditorsValue(ui, property, valueParams);
        }
    }
}

int FSCameraSettingsDialog::insertIndexForPresetName(const QString &presetName) const
{
    const int count = ui->comboBoxPresets->count();
    for (int i = 0; i < count; i++) {
        if (ui->comboBoxPresets->itemText(i) > presetName) {
            return i;
        }
    }

    if (count != 0) {
        return (count - 1);
    }

    return 0;
}

void FSCameraSettingsDialog::createNewPreset()
{
    if (d->mode != FSCameraSettingsDialog::ChangePresetMode)
        return;

    bool isInserted = false;

    for (ushort newPresetNumber = 1; newPresetNumber < MAX_FS_CAMERA_FREE_PRESET_COUNT; newPresetNumber++) {
        const QString newPresetName = STRING_PRESET.arg(newPresetNumber);

        if ( d->umapCameraUserPresets.find(newPresetName) == d->umapCameraUserPresets.end() &&
             ui->comboBoxPresets->findText(newPresetName) == -1 ) {
            saveCurrentPresetValues();

            d->currentPresetName = newPresetName;
            d->umapCameraUserPresets.insert( { newPresetName, FSCameraPropertyValuesUMap() } );
            ui->comboBoxPresets->blockSignals(true);
            ui->comboBoxPresets->insertItem(insertIndexForPresetName(newPresetName), newPresetName);
            ui->comboBoxPresets->setCurrentIndex(ui->comboBoxPresets->findText(newPresetName));
            ui->comboBoxPresets->blockSignals(false);
            restoreDefaultValues();
            isInserted = true;
            break;
        }
    }

    if (!isInserted) {
        ui->comboBoxPresets->blockSignals(true);
        ui->comboBoxPresets->setCurrentIndex(ui->comboBoxPresets->findText(d->currentPresetName));
        ui->comboBoxPresets->blockSignals(false);
        restoreDefaultValues();
    }
}

void FSCameraSettingsDialog::updatePresetsFromCameraStorage()
{
    d->currentPresetName.clear();
    d->umapCameraUserPresets.clear();

    ui->comboBoxPresets->blockSignals(true);
    ui->comboBoxPresets->clear();
    ui->comboBoxPresets->blockSignals(false);

    if (d->mode == FSCameraSettingsDialog::LockPropertiesMode) {
        ui->comboBoxPresets->blockSignals(true);

        std::deque<QString> dequePresetNames;
        FSCamerasStorage *camerasStorage = qobject_cast<FSCamerasStorage *>(d->camera->camerasStorage());
        if (camerasStorage) {
            const std::vector<QString> vectorPresetNames = camerasStorage->getCameraUserPresetNames(d->camera->devicePath());
            for (const QString &presetName: vectorPresetNames) {
                dequePresetNames.push_back(presetName);
            }
        }

        sort(dequePresetNames.begin(), dequePresetNames.end());

        if (d->mode == FSCameraSettingsDialog::LockPropertiesMode) {
            if ( d->lockPropertiesManager &&
                 d->lockPropertiesManager->isContaintsManualLockProperties(d->camera->devicePath()) )
                dequePresetNames.push_front(MANUAL_PRESET);
        }

        for (const QString &presetName : dequePresetNames) {
            ui->comboBoxPresets->addItem(presetName);
        }

        ui->comboBoxPresets->blockSignals(false);

        updateCurrentLockPresetIndex();
    } else if (d->mode == FSCameraSettingsDialog::ChangePresetMode) {
        FSCamerasStorage *camerasStorage = qobject_cast<FSCamerasStorage *>(d->camera->camerasStorage());
        if (camerasStorage) {
            d->umapCameraUserPresets = camerasStorage->getCameraUserPresets(d->camera->devicePath());
        }

        ui->comboBoxPresets->blockSignals(true);

        ui->comboBoxPresets->clear();

        std::vector<QString> vectorPresetNames;
        vectorPresetNames.reserve(d->umapCameraUserPresets.size());
        for (FSCameraUserPresetsUMap::const_iterator iterator = d->umapCameraUserPresets.begin();
             iterator != d->umapCameraUserPresets.end();
             ++iterator) {
            vectorPresetNames.push_back(iterator->first);
        }

        sort(vectorPresetNames.begin(), vectorPresetNames.end());

        for (const QString &presetName : vectorPresetNames) {
            ui->comboBoxPresets->addItem(presetName);
        }

        ui->comboBoxPresets->addItem(TR_NEW_PRESET);

        ui->comboBoxPresets->setCurrentIndex(0);
        ui->comboBoxPresets->blockSignals(false);

        if (!d->umapCameraUserPresets.empty())
            restorePresetValues(0);
        else
            createNewPreset();
    }
}

bool FSCameraSettingsDialog::renameCurrentPresetName(const QString &newPresetName)
{
    if (d->currentPresetName == newPresetName)
        return true;

    if (d->currentPresetName.isEmpty() || newPresetName.isEmpty())
        return false;

    if ( newPresetName == TR_NEW_PRESET ||
         ui->comboBoxPresets->findText(newPresetName) != -1 ) {
        QMessageBox::critical(this,
                              tr("Invalid preset names"),
                              tr("There is already such a name \"%1\"!").arg(newPresetName));
        return false;
    }

    auto nh = d->umapCameraUserPresets.extract(d->currentPresetName);

    if (!nh.empty())
        nh.key() = newPresetName;

    const QString oldPresetName = d->currentPresetName;
    d->currentPresetName = newPresetName;

    ui->comboBoxPresets->blockSignals(true);
    ui->comboBoxPresets->removeItem(ui->comboBoxPresets->findText(oldPresetName));
    const int insertIndex = insertIndexForPresetName(newPresetName);
    ui->comboBoxPresets->insertItem(insertIndex, newPresetName);
    ui->comboBoxPresets->setCurrentIndex(insertIndex);
    ui->comboBoxPresets->blockSignals(false);

    restorePresetValues(ui->comboBoxPresets->findText(newPresetName));

    return true;
}

void FSCameraSettingsDialog::updateLabelSelection(FSCameraProperty property)
{
    if (!d->camera)
        return;

    bool isLock = false;

    if (d->lockPropertiesManager && d->lockPropertiesManager->isLockedProperty(d->camera->devicePath(), property))
        isLock = true;

    d->setLabelSelection(d->getLabel(ui, property), d->getEditorValue(ui, property), d->getDefaultValue(property), isLock);
}

void FSCameraSettingsDialog::retranslate()
{
    ui->retranslateUi(this);

    d->origWindowTitle = windowTitle();
    updateWindowTitle();

    retranslatePropertyNames();

    if (d->mode == FSCameraSettingsDialog::LockPropertiesMode) {
        if ( ui->comboBoxPresets->count() != 0 &&
             d->lockPropertiesManager &&
             d->lockPropertiesManager->isContaintsManualLockProperties(d->camera->devicePath()) ) {
            ui->comboBoxPresets->setItemText(0, MANUAL_PRESET);
        }
    } else if (d->mode == FSCameraSettingsDialog::ChangePresetMode) {
        if (ui->comboBoxPresets->count() != 0) {
            ui->comboBoxPresets->setItemText(ui->comboBoxPresets->count() - 1, TR_NEW_PRESET);
        }
    }

    // Update Powerline Frequency ComboBox Editor
    d->setEditorsRange(ui, FSCameraProperty::PowerlineFrequency, d->getRangeParams(PowerlineFrequency));
}

void FSCameraSettingsDialog::updateMinimumWidthLabels()
{
    for (const FSCameraProperty &property : fsAllCameraProperties()) {
        QLabel *label = d->getLabel(ui, property);

        if (label) {
            const QFont oldFont = label->font();
            QFont font = label->font();
            font.setBold(true);
            font.setItalic(true);
            label->setFont(font);
            label->adjustSize();
            label->setMinimumWidth(label->width());
            label->setFont(oldFont);
            label->adjustSize();
        }
    }
}

void FSCameraSettingsDialog::updateMinimumWidthSpinBox()
{
    for (const FSCameraProperty &property : fsAllCameraProperties()) {
        QSpinBox *spinBox = d->getSpinBox(ui, property);

        if (spinBox) {
            const int oldMinValue = spinBox->minimum();
            const int oldMaxValue = spinBox->maximum();
            const int oldValue = spinBox->value();
            spinBox->setRange(0, 999999);
            spinBox->setValue(999999);
            spinBox->adjustSize();
            spinBox->setMinimumWidth(spinBox->width());
            spinBox->setRange(oldMinValue, oldMaxValue);
            spinBox->setValue(oldValue);
            spinBox->adjustSize();
        }
    }
}

void FSCameraSettingsDialog::readRangesFromDevice()
{
    for (const FSCameraProperty &property : fsAllCameraProperties())
        d->readRangesFromDevice(ui, property);
}

void FSCameraSettingsDialog::updateValues()
{
    for (const auto& [property, rangeParams] : d->umapCameraPropertyRangeParams)
        d->updateValue(ui, property);
}

void FSCameraSettingsDialog::restoreDefaultValues()
{
    for (const auto& [property, rangeParams] : d->umapCameraPropertyRangeParams) {
        FSValueParams defaultValueParams = d->getDefaultValue(property);

        if (defaultValueParams.isNull())
            continue;

        if (d->isCameraValueSendEnable) {
            if (d->camera->set(property, defaultValueParams)) {
                d->updateValue(ui, property);
            } else {
                qCritical("%s: Failed to set default value(value:\"%ld\", flags:\"%ld\") for property \"%s\", devicePath=\"%s\"!",
                          FSCameraSettingsDialog::staticMetaObject.className(),
                          defaultValueParams.value(),
                          defaultValueParams.flags(),
                          fsGetEnumName(property).toLocal8Bit().constData(),
                          d->camera->devicePath().toLocal8Bit().constData());
            }
        } else {
            d->setEditorsValue(ui, property, defaultValueParams);
        }
    }
}

void FSCameraSettingsDialog::lockedProperty(const DevicePath &devicePath,
                                            FSCameraProperty property)
{
    setLockProperty(devicePath, property, true);
}

void FSCameraSettingsDialog::unlockedProperty(const DevicePath &devicePath,
                                              FSCameraProperty property)
{
    setLockProperty(devicePath, property, false);
}

void FSCameraSettingsDialog::applyCurrentValueParams(int editorTypeProperty)
{
    const FSCameraSettingsDialogPrivate::EditorType editorType = d->getEditorType(editorTypeProperty);
    const FSCameraProperty property = d->getProperty(editorTypeProperty);
    FSValueParams valueParams = d->getEditorValue(ui, property, editorType);

    updateLabelSelection(property);

    if (!valueParams.isNull() && !valueParams.isAutoControl()) {
        if (editorType == FSCameraSettingsDialogPrivate::SliderEditorType) {
            QSpinBox *spinBox = d->getSpinBox(ui, property);
            if (spinBox) {
                spinBox->blockSignals(true);
                spinBox->setValue(d->valueForSpinBox(valueParams.value(), d->getRangeParams(property)));
                spinBox->blockSignals(false);
            }
        } else if (editorType == FSCameraSettingsDialogPrivate::SpinBoxEditorType) {
            QSlider *slider = d->getSlider(ui, property);
            if (slider) {
                slider->blockSignals(true);
                slider->setValue(d->valueForSlider(valueParams.value(), d->getRangeParams(property)));
                slider->blockSignals(false);
            }
        }
    }

    if (isCameraValueSendEnable()) {
        if (FSSettings::isCameraValueUpdateEnable())
            setCameraValueUpdateEnable(false);

        d->sendValueParams(ui, property, editorType);

        if (FSSettings::isCameraValueUpdateEnable())
            setCameraValueUpdateEnable(true);
    }
}

void FSCameraSettingsDialog::sendLockCameraProperty(int property)
{
    if (d->mode != FSCameraSettingsDialog::LockPropertiesMode || !d->lockPropertiesManager)
        return;

    const FSCameraProperty fsProperty = FSCameraProperty(property);

    QCheckBox *checkBox = d->getLockCheckBox(ui, fsProperty);

    if (checkBox) {
        if (checkBox->isChecked())
            d->lockPropertiesManager->manualLockProperty(d->camera->devicePath(), fsProperty);
        else
            d->lockPropertiesManager->manualUnlockProperty(d->camera->devicePath(), fsProperty);
    }
}

void FSCameraSettingsDialog::updateCurrentLockPresetIndex(const DevicePath &devicePath)
{
    if (devicePath != d->camera->devicePath())
        return;

    updateCurrentLockPresetIndex();
}

void FSCameraSettingsDialog::updateCurrentPresetRemoveButton()
{
    if (d->mode != FSCameraSettingsDialog::ChangePresetMode)
        return;

    if (ui->comboBoxPresets->currentIndex() == -1) {
        ui->toolButtonRemoveCurrentPreset->setEnabled(false);
        return;
    } else {
        if (d->currentPresetName == TR_NEW_PRESET) {
            ui->toolButtonRemoveCurrentPreset->setEnabled(false);
            return;
        }
    }

    ui->toolButtonRemoveCurrentPreset->setEnabled(true);
}

void FSCameraSettingsDialog::restorePresetValues(int comboBoxIndex)
{
    if (d->mode == FSCameraSettingsDialog::LockPropertiesMode) {
        if (d->lockPropertiesManager) {
            if (ui->comboBoxPresets->currentIndex() == ui->comboBoxPresets->findText(MANUAL_PRESET)) {
                d->lockPropertiesManager->presetUnlockProperies(d->camera->devicePath());
            } else {
                d->lockPropertiesManager->presetLockProperties(d->camera->devicePath(),
                                                               ui->comboBoxPresets->itemText(ui->comboBoxPresets->currentIndex()));
            }
        }
    } else if (d->mode == FSCameraSettingsDialog::ChangePresetMode) {
        if (comboBoxIndex == -1) {
            restoreDefaultValues();
            return;
        }

        const QString &currentPresetName = ui->comboBoxPresets->itemText(comboBoxIndex);

        if (currentPresetName == TR_NEW_PRESET) {
            createNewPreset();
        } else {
            saveCurrentPresetValues();
            d->currentPresetName = currentPresetName;
            restorePresetValues(currentPresetName);
        }
    }
}

void FSCameraSettingsDialog::removeCurrentPreset()
{
    const int currentComboBoxIndex = ui->comboBoxPresets->currentIndex();

    if (currentComboBoxIndex == -1)
        return;

    const QString &currentPresetName = ui->comboBoxPresets->itemText(currentComboBoxIndex);

    if (currentPresetName == TR_NEW_PRESET)
        return;

    if (currentComboBoxIndex - 1 >= 0)
        ui->comboBoxPresets->setCurrentIndex(currentComboBoxIndex - 1);
    else if (ui->comboBoxPresets->count() != 0)
        ui->comboBoxPresets->setCurrentIndex(currentComboBoxIndex + 1);
    else
        createNewPreset();

    FSCameraUserPresetsUMap::const_iterator iterator = d->umapCameraUserPresets.find(currentPresetName);
    if (iterator != d->umapCameraUserPresets.end()) {
        d->umapCameraUserPresets.erase(iterator);
    }

    ui->comboBoxPresets->blockSignals(true);
    ui->comboBoxPresets->removeItem(currentComboBoxIndex);
    ui->comboBoxPresets->blockSignals(false);
}

void FSCameraSettingsDialog::execRenameCurrentPresetName()
{
    FSRenamePresetNameDialog dialog;
    dialog.setPresetName(d->currentPresetName);

dialog_exec:
    int res = dialog.exec();

    if (res == QDialog::Accepted) {
        if (!renameCurrentPresetName(dialog.presetName())) {
            goto dialog_exec;
        }
    }
}
