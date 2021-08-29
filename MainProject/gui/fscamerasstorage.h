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

#include <WebCamFS/CoreCamerasStorage>

typedef std::unordered_map<FSCameraProperty, FSValueParams> FSCameraPropertyValuesUMap;
typedef std::unordered_map<FSCameraProperty, FSRangeParams> FSCameraPropertyRangesUMap;

typedef std::unordered_map<DevicePath, FSCameraPropertyValuesUMap> FSCameraPathValuesUMap;
typedef std::unordered_map<DevicePath, FSCameraPropertyRangesUMap> FSCameraPathRangesUMap;

typedef std::unordered_map<QString, FSCameraPropertyValuesUMap> FSCameraUserPresetsUMap;
typedef std::unordered_map<DevicePath, FSCameraUserPresetsUMap> FSCameraPathUserPresetsUMap;

class FSCamerasStoragePrivate;

class FSCamerasStorage : public FSCoreCamerasStorage
{
    Q_OBJECT

public:
    explicit FSCamerasStorage(QObject *parent = nullptr);
    ~FSCamerasStorage() override;

    // Camera user default names
    void setUserDefaultValues(const FSCameraPathValuesUMap &umapCameraPathValues);
    void setUserDefaultValues(const DevicePath &devicePath, const FSCameraPropertyValuesUMap &umapCameraPropertyValues);
    void setUserDefaultValue(const DevicePath &devicePath, FSCameraProperty property, const FSValueParams &valueParams);

    bool isUserDefaultValuesUsed(const DevicePath &devicePath);
    bool isUserDefaultValueUsed(const DevicePath &devicePath, FSCameraProperty property);

    FSCameraPathValuesUMap getUserDefaultValues() const;
    FSCameraPropertyValuesUMap getUserDefaultValues(const DevicePath &devicePath) const;
    FSValueParams getUserDefaultValue(const DevicePath &devicePath, FSCameraProperty property) const;

    void userDefaultValuesRemove(const DevicePath &devicePath);
    void userDefaultValueRemove(const DevicePath &devicePath, FSCameraProperty property);

    void userDefaultValuesClear();

    void saveDefaultValueParams();
    void loadDefaultValueParams();


    // Camera user presets
    void setCameraUserPresets(const FSCameraPathUserPresetsUMap &umapCameraUserPresets);
    void setCameraUserPresets(const DevicePath &devicePath, const FSCameraUserPresetsUMap &umapCameraUserPresets);
    void setCameraUserPreset(const DevicePath &devicePath, const QString &presetName, const FSCameraPropertyValuesUMap &umapCameraPropertyValues);
    void setCameraUserPreset(const DevicePath &devicePath, const QString &presetName, FSCameraProperty property, const FSValueParams &valueParams);

    bool isCameraUserPresetsUsed(const DevicePath &devicePath);
    bool isCameraUserPresetUsed(const DevicePath &devicePath, const QString &presetName);
    bool isCameraUserPresetValueUsed(const DevicePath &devicePath, const QString &presetName, FSCameraProperty property);

    FSCameraPathUserPresetsUMap getCameraUserPresets() const;
    std::vector<QString> getCameraUserPresetNames(const DevicePath &devicePath) const;
    FSCameraUserPresetsUMap getCameraUserPresets(const DevicePath &devicePath) const;
    FSCameraPropertyValuesUMap getCameraUserPreset(const DevicePath &devicePath, const QString &presetName) const;
    FSValueParams getCameraUserPresetValue(const DevicePath &devicePath, const QString &presetName, FSCameraProperty property) const;

    void cameraUserPresetsRemove(const DevicePath &devicePath);
    void cameraUserPresetRemove(const DevicePath &devicePath, const QString &presetName);
    void cameraUserPresetValueRemove(const DevicePath &devicePath, const QString &presetName, FSCameraProperty property);

    void cameraUserPresetsClear();

    void saveCameraUserPresets();
    void loadCameraUserPresets();

public slots:
    void saveAll() override;
    void loadAll() override;

signals:
    void cameraUserPresetNamesChanged(const DevicePath &devicePath);

private:
    FSCamerasStoragePrivate *d;

    void init();
};
