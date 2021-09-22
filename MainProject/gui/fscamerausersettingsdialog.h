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

#include <WebCamFS/Structs>

class FSCamerasStorage;
class FSLockPropertiesManager;

namespace Ui { class FSCameraUserSettingsDialog; }

class FSCameraUserSettingsDialogPrivate;

class FSCameraUserSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FSCameraUserSettingsDialog(QWidget *parent = nullptr);
    ~FSCameraUserSettingsDialog() override;

    void setCamerasStorage(FSCamerasStorage *camerasStorage);
    FSCamerasStorage *camerasStorage() const;

    void setLockPropertiesManager(FSLockPropertiesManager *lockPropertiesManager);
    FSLockPropertiesManager *lockPropertiesManager() const;

signals:
    void showCameraSettingsDialog(const DevicePath &devicePath);

private:
    FSCameraUserSettingsDialogPrivate *d;
    Ui::FSCameraUserSettingsDialog *ui;

    void init();

private slots:
    void retranslate();

    void updateTabData(int index);

    void updateButtonsChangeCameraUserSettings();
    void updateButtonsChangeDefaultSettings();
    void updateButtonsChangePresets();
    void updateButtonsLockProperties();

    void on_pushButtonChangeCameraUserSettings();
    void on_pushButtonChangeDefaultSettings();
    void on_pushButtonChangePresets();
    void on_pushButtonChangeLockProperties();

    void on_pushButtonClearCameraUserSettings();
    void on_pushButtonClearDefaultSettings();
    void on_pushButtonClearPresets();
    void on_pushButtonClearLockProperties();

    void execCameraUserSettingsDialog(const QModelIndex &index);
    void execDefaultSettingsDialog(const QModelIndex &index);
    void execPresetSettingsDialog(const QModelIndex &index);
    void execLockPropertiesDialog(const QModelIndex &index);
};
