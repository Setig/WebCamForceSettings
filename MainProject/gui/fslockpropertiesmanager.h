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

    bool isLockedLeastOneProperty(FSCamera *camera) const;
    bool isLockedProperty(FSCamera *camera, FSCameraProperty property) const;
    FSValueParams lockedValueParams(FSCamera *camera, FSCameraProperty property) const;

    bool isContaintsManualLockProperties(FSCamera *camera) const;
    bool isContaintsPresetLock(FSCamera *camera) const;

    enum LockMode {
        NoneLockMode = 0,
        ManualLockMode,
        PresetLockMode
    };

    LockMode currentLockMode(FSCamera *camera) const;

    QString lockPresetName(FSCamera *camera) const;

    void setLockPropertiesInterval(int msec);
    int lockPropertiesInterval() const;

    void loadLockSettings();
    void saveLockSettings();

    bool isLockProperiesPause() const;
    void lockProperiesPause(int msec);
    void lockProperiesPauseAbort();

public slots:
    void lockProperiesPauseFinish();

    bool manualLockProperty(FSCamera *camera, FSCameraProperty property);
    bool manualLockProperties(FSCamera *camera, const std::vector<FSCameraProperty> &vecProperties);

    void manualLockProperty(FSCamera *camera,
                            FSCameraProperty property,
                            const FSValueParams &valueParams);

    void manualUnlockProperty(FSCamera *camera, FSCameraProperty property);
    void manualUnlockProperties(FSCamera *camera, const std::vector<FSCameraProperty> &vecProperties);
    void manualUnlockProperties(FSCamera *camera);

    void presetLockProperties(FSCamera *camera, const QString &presetName);
    void presetUnlockProperies(FSCamera *camera);

signals:
    void lockedCamera(FSCamera *camera);
    void unlockedCamera(FSCamera *camera);

    void lockedProperty(FSCamera *camera, FSCameraProperty property);
    void unlockedProperty(FSCamera *camera, FSCameraProperty property);

    void switchedToManualMode(FSCamera *camera);

    void lockedPreset(FSCamera *camera, const QString &presetName);
    void unlockedPreset(FSCamera *camera);

    void lockPropertiesEnableChanged(bool isEnable);

private:
    FSLockPropertiesManagerPrivate *d;

    void init();

    void updateTimerStatus();

    void copyCurrentPresetToManualPropertyValues(const DevicePath &devicePath);

    void trySwitchToManualMode(FSCamera *camera);
    void switchToManualMode(FSCamera *camera);

    bool setLockValueParams(FSCamera *camera,
                            FSCameraProperty property,
                            const FSValueParams &lockedValueParams);

    friend class FSLockPropertiesManagerPrivate;

private slots:
    void addedCamera(const DevicePath &devicePath);
    void removedCamera(const DevicePath &devicePath);

    void updateCurrentPreset(const DevicePath &devicePath);

    void lockProperiesPauseTimeout();

    void sendAllLockedValues();
};
