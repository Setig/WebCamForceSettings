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

#pragma once

#include <QDialog>

#include <WebCamFS/Camera>

namespace Ui { class FSCameraSettingsDialog; }

class FSLockPropertiesManager;

class FSCameraSettingsDialogPrivate;

class FSCameraSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FSCameraSettingsDialog(FSCamera *camera,
                                    QWidget *parent = nullptr);
    ~FSCameraSettingsDialog() override;

    FSCamera *camera() const;

    enum Mode {
        LockPropertiesMode = 0,
        ChangeDefaultValuesMode,
        ChangePresetMode
    };

    void setMode(Mode mode);
    Mode mode() const;

    void setLockPropertiesManager(FSLockPropertiesManager *lockPropertiesManager);
    FSLockPropertiesManager *lockPropertiesManager() const;

    void setCameraValueUpdateEnable(bool isEnable);
    bool isCameraValueUpdateEnable() const;

    void setCameraValueUpdateInterval(int msec);
    int cameraValueUpdateInterval() const;

    void setCameraValueSendEnable(bool isEnable);
    bool isCameraValueSendEnable() const;

    void updateWindowTitle();

    void updateValueParams(FSCameraProperty property);
    void setValueParams(FSCameraProperty property, const FSValueParams &valueParams);

    bool isValueEnable(FSCameraProperty property) const;
    FSValueParams valueParams(FSCameraProperty property) const;

    void sendValuesToCameraStorage();

public Q_SLOTS:
    void done(int r) override;

protected:
    void keyPressEvent(QKeyEvent *event) override;

    bool eventFilter(QObject *object, QEvent *event) override;

private:
    FSCameraSettingsDialogPrivate *d;
    Ui::FSCameraSettingsDialog *ui;

    void init(FSCamera *camera);
    void initEditorsConnections();

    void retranslatePropertyNames();

    void updateModeParams();

    void updateWidgetsByMode();

    void setLockProperty(FSCamera *camera,
                         FSCameraProperty property,
                         bool isLock);

    void saveCurrentPresetValues();
    void restorePresetValues(const QString &presetName);

    int insertIndexForPresetName(const QString &presetName) const;

    void createNewPreset();

    void updatePresetsFromCameraStorage();

    bool renameCurrentPresetName(const QString &newPresetName);

    void updateLabelSelection(FSCameraProperty property);

private slots:
    void retranslate();

    void updateMinimumWidthLabels();
    void updateMinimumWidthSpinBox();

    void readRangesFromDevice();
    void updateValues();
    void restoreDefaultValues();

    void lockedProperty(FSCamera *camera, FSCameraProperty property);
    void unlockedProperty(FSCamera *camera, FSCameraProperty property);

    void applyCurrentValueParams(int editorTypeProperty);
    void sendLockCameraProperty(int property);

    void updateCurrentPresetRemoveButton();
    void restorePresetValues(int comboBoxIndex);
    void removeCurrentPreset();

    void execRenameCurrentPresetName();
};
