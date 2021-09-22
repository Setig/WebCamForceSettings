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

#include <QObject>

#include <WebCamFS/Camera>
#include <WebCamFS/CamerasStorage>

class FSCamerasStorage;

class FSLockPropertiesManagerPrivate;

class FSLockPropertiesManager : public QObject
{
    Q_OBJECT

public:
    explicit FSLockPropertiesManager(QObject *parent = nullptr);
    ~FSLockPropertiesManager() override;

    void setCamerasStorage(FSCamerasStorage *camerasStorage);
    FSCamerasStorage *camerasStorage() const;

    bool isExistLockedProperties() const;

    bool isLockPropertiesEnable() const;
    void setLockPropertiesEnable(bool isEnable);

    bool isLockedLeastOneProperty(const DevicePath &devicePath) const;
    bool isLockedProperty(const DevicePath &devicePath,
                          FSCameraProperty property) const;
    FSValueParams lockedValueParams(const DevicePath &devicePath,
                                    FSCameraProperty property) const;

    bool isContaintsManualLockProperties(const DevicePath &devicePath) const;
    bool isContaintsPresetLock(const DevicePath &devicePath) const;

    std::vector<DevicePath> lockedDevicePaths() const;

    FSCameraPropertyValuesUMap lockedProperties(const DevicePath &devicePath) const;

    enum LockMode {
        NoneLockMode = 0,
        ManualLockMode,
        PresetLockMode
    };

    LockMode currentLockMode(const DevicePath &devicePath) const;

    QString lockPresetName(const DevicePath &devicePath) const;

    void loadLockSettings();
    void saveLockSettings();

    bool isLockProperiesPause() const;
    void lockProperiesPause(int msec);
    void lockProperiesPauseAbort();

public slots:
    void lockProperiesPauseFinish();

    bool manualLockProperty(const DevicePath &devicePath,
                            FSCameraProperty property);
    bool manualLockProperties(const DevicePath &devicePath,
                              const std::vector<FSCameraProperty> &vecProperties);

    void manualLockProperty(const DevicePath &devicePath,
                            FSCameraProperty property,
                            const FSValueParams &valueParams);

    void manualUnlockProperty(const DevicePath &devicePath,
                              FSCameraProperty property);
    void manualUnlockProperties(const DevicePath &devicePath,
                                const std::vector<FSCameraProperty> &vecProperties);
    void manualUnlockProperties(const DevicePath &devicePath);

    void presetLockProperties(const DevicePath &devicePath,
                              const QString &presetName);
    void presetUnlockProperies(const DevicePath &devicePath);

    void updateTimerStatus();

signals:
    void lockedCamera(const DevicePath &devicePath);
    void unlockedCamera(const DevicePath &devicePath);

    void lockedProperty(const DevicePath &devicePath, FSCameraProperty property);
    void unlockedProperty(const DevicePath &devicePath, FSCameraProperty property);

    void switchedToManualMode(const DevicePath &devicePath);

    void lockedPreset(const DevicePath &devicePath, const QString &presetName);
    void unlockedPreset(const DevicePath &devicePath);

    void lockPropertiesEnableChanged(bool isEnable);

private:
    FSLockPropertiesManagerPrivate *d;

    void init();

    void copyCurrentPresetToManualPropertyValues(const DevicePath &devicePath);

    void trySwitchToManualMode(const DevicePath &devicePath);
    void switchToManualMode(const DevicePath &devicePath);

    bool setLockValueParams(const DevicePath &devicePath,
                            FSCameraProperty property,
                            const FSValueParams &lockedValueParams);

    void restoreDefaultPropertyValue(const DevicePath &devicePath,
                                     FSCameraProperty property);

    friend class FSLockPropertiesManagerPrivate;

private slots:
    void addCamera(const DevicePath &devicePath);
    void removeCamera(const DevicePath &devicePath);

    void updateCurrentPreset(const DevicePath &devicePath);

    void lockProperiesPauseTimeout();

    void sendAllLockedValues();
};
