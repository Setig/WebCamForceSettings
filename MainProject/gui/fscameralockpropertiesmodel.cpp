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

#include "fscameralockpropertiesmodel.h"

#include <unordered_set>

#include <WebCamFS/CamerasStorage>
#include <WebCamFS/TranslationsHelper>
#include <WebCamFS/LockPropertiesManager>

class FSCameraLockPropertiesModelPrivate
{
public:
    FSCameraLockPropertiesModelPrivate();

    FSLockPropertiesManager *lockPropertiesManager;
};

FSCameraLockPropertiesModelPrivate::FSCameraLockPropertiesModelPrivate()
    : lockPropertiesManager(nullptr)
{
    // do nothing
}

FSCameraLockPropertiesModel::FSCameraLockPropertiesModel(QObject *parent)
    : FSAbstractCameraSettingsModel(parent)
{
    init();
}

FSCameraLockPropertiesModel::~FSCameraLockPropertiesModel()
{
    delete d;
    d = nullptr;
}

void FSCameraLockPropertiesModel::setLockPropertiesManager(FSLockPropertiesManager *lockPropertiesManager)
{
    if (d->lockPropertiesManager != lockPropertiesManager) {
        if (d->lockPropertiesManager) {
            disconnect(d->lockPropertiesManager, &FSLockPropertiesManager::lockedCamera,
                       this,                     &FSCameraLockPropertiesModel::updateLockCameraColumns);
            disconnect(d->lockPropertiesManager, &FSLockPropertiesManager::unlockedCamera,
                       this,                     &FSCameraLockPropertiesModel::updateUnlockCameraColumns);

            disconnect(d->lockPropertiesManager, &FSLockPropertiesManager::lockedProperty,
                       this,                     &FSCameraLockPropertiesModel::updateLockPropertyColumns);
            disconnect(d->lockPropertiesManager, &FSLockPropertiesManager::unlockedProperty,
                       this,                     &FSCameraLockPropertiesModel::updateUnlockPropertyColumns);

            disconnect(d->lockPropertiesManager, &FSLockPropertiesManager::switchedToManualMode,
                       this,                     &FSCameraLockPropertiesModel::updateSwitchToManualModeColumns);
            disconnect(d->lockPropertiesManager, &FSLockPropertiesManager::lockedPreset,
                       this,                     &FSCameraLockPropertiesModel::updateLockPresetColumns);
            disconnect(d->lockPropertiesManager, &FSLockPropertiesManager::unlockedPreset,
                       this,                     &FSCameraLockPropertiesModel::updateUnlockPresetColumns);
        }

        d->lockPropertiesManager = lockPropertiesManager;

        if (d->lockPropertiesManager) {
            connect(d->lockPropertiesManager, &FSLockPropertiesManager::lockedCamera,
                    this,                     &FSCameraLockPropertiesModel::updateLockCameraColumns);
            connect(d->lockPropertiesManager, &FSLockPropertiesManager::unlockedCamera,
                    this,                     &FSCameraLockPropertiesModel::updateUnlockCameraColumns);

            connect(d->lockPropertiesManager, &FSLockPropertiesManager::lockedProperty,
                    this,                     &FSCameraLockPropertiesModel::updateLockPropertyColumns);
            connect(d->lockPropertiesManager, &FSLockPropertiesManager::unlockedProperty,
                    this,                     &FSCameraLockPropertiesModel::updateUnlockPropertyColumns);

            connect(d->lockPropertiesManager, &FSLockPropertiesManager::switchedToManualMode,
                    this,                     &FSCameraLockPropertiesModel::updateSwitchToManualModeColumns);
            connect(d->lockPropertiesManager, &FSLockPropertiesManager::lockedPreset,
                    this,                     &FSCameraLockPropertiesModel::updateLockPresetColumns);
            connect(d->lockPropertiesManager, &FSLockPropertiesManager::unlockedPreset,
                    this,                     &FSCameraLockPropertiesModel::updateUnlockPresetColumns);
        }

        updateDevices();
    }
}

FSLockPropertiesManager *FSCameraLockPropertiesModel::lockPropertiesManager() const
{
    return d->lockPropertiesManager;
}

QVariant FSCameraLockPropertiesModel::getDeviceData(const DevicePath &devicePath,
                                                    int column,
                                                    int role) const
{
    if (role == Qt::TextAlignmentRole) {
        switch (column) {
        case NameColumn:
            return Qt::AlignLeft;
        case IsConnectedColumn:
            return Qt::AlignCenter;
        case CurrentPresetColumn:
            return Qt::AlignLeft;
        case ManualPropertiesCountColumn:
            return Qt::AlignCenter;
        case PresetsCountColumn:
            return Qt::AlignCenter;
        default:
            break;
        }

        return QVariant();
    }

    if (!d->lockPropertiesManager)
        return QVariant();

    switch (column) {
    case NameColumn:
        return camerasStorage()->getCameraDisplayName(devicePath);
    case IsConnectedColumn:
        return camerasStorage()->isCameraConnected(devicePath);
    case CurrentPresetColumn:
    {
        switch (d->lockPropertiesManager->currentLockMode(devicePath)) {
        case FSLockPropertiesManager::NoneLockMode:
            return tr("None");
        case FSLockPropertiesManager::ManualLockMode:
            return tr("Manual");
        case FSLockPropertiesManager::PresetLockMode:
            return d->lockPropertiesManager->lockPresetName(devicePath);
        }

        break;
    }
    case ManualPropertiesCountColumn:
        return d->lockPropertiesManager->lockedProperties(devicePath).size();
    case PresetsCountColumn:
        return camerasStorage()->getCameraUserPresetNames(devicePath).size();
    default:
        break;
    }

    return QVariant();
}

std::vector<DevicePath> FSCameraLockPropertiesModel::getDevicesFromStorage() const
{
    std::vector<DevicePath> result;

    FSCamerasStorage *camerasStorage = this->camerasStorage();
    if (camerasStorage && d->lockPropertiesManager) {
        const std::vector<DevicePath> vecLockedDevicePaths    = d->lockPropertiesManager->lockedDevicePaths();
        const std::vector<DevicePath> vecAvailableDevicePaths = camerasStorage->availableDevicePaths();

        result.reserve(vecLockedDevicePaths.size() + vecAvailableDevicePaths.size());

        std::unordered_set<DevicePath> usetLockedDevicePaths;
        usetLockedDevicePaths.reserve(vecLockedDevicePaths.size());

        for (const DevicePath &devicePath : vecLockedDevicePaths) {
            result.push_back(devicePath);
            usetLockedDevicePaths.insert(devicePath);
        }

        for (const DevicePath &devicePath : vecAvailableDevicePaths) {
            if ( !camerasStorage->isContaintsBlackList(devicePath) &&
                 usetLockedDevicePaths.find(devicePath) == usetLockedDevicePaths.end() ) {
                result.push_back(devicePath);
            }
        }
    }

    return result;
}

void FSCameraLockPropertiesModel::addCamera(const DevicePath &devicePath)
{
    if ( d->lockPropertiesManager &&
         !d->lockPropertiesManager->isLockedLeastOneProperty(devicePath) ) {
        addDevice(devicePath);
    } else {
        emitDataChanged(devicePath, { NameColumn, IsConnectedColumn });
    }
}

void FSCameraLockPropertiesModel::removeCamera(const DevicePath &devicePath)
{
    if ( d->lockPropertiesManager &&
         !d->lockPropertiesManager->isLockedLeastOneProperty(devicePath) ) {
        removeDevice(devicePath);
    } else {
        emitDataChanged(devicePath, { NameColumn, IsConnectedColumn });
    }
}

void FSCameraLockPropertiesModel::updateLockCameraColumns(const DevicePath &devicePath)
{
    emitDataChanged(devicePath, { CurrentPresetColumn, ManualPropertiesCountColumn, PresetsCountColumn });
}

void FSCameraLockPropertiesModel::updateUnlockCameraColumns(const DevicePath &devicePath)
{
    emitDataChanged(devicePath, { CurrentPresetColumn, ManualPropertiesCountColumn, PresetsCountColumn });
}

void FSCameraLockPropertiesModel::updateLockPropertyColumns(const DevicePath &devicePath,
                                                            FSCameraProperty property)
{
    Q_UNUSED(property);

    emitDataChanged(devicePath, { CurrentPresetColumn, ManualPropertiesCountColumn, PresetsCountColumn });
}

void FSCameraLockPropertiesModel::updateUnlockPropertyColumns(const DevicePath &devicePath,
                                                              FSCameraProperty property)
{
    Q_UNUSED(property);

    emitDataChanged(devicePath, { CurrentPresetColumn, ManualPropertiesCountColumn, PresetsCountColumn });
}

void FSCameraLockPropertiesModel::updateSwitchToManualModeColumns(const DevicePath &devicePath)
{
    emitDataChanged(devicePath, { CurrentPresetColumn, ManualPropertiesCountColumn, PresetsCountColumn });
}

void FSCameraLockPropertiesModel::updateLockPresetColumns(const DevicePath &devicePath,
                                                          const QString &presetName)
{
    Q_UNUSED(presetName);

    emitDataChanged(devicePath, { CurrentPresetColumn, ManualPropertiesCountColumn, PresetsCountColumn });
}

void FSCameraLockPropertiesModel::updateUnlockPresetColumns(const DevicePath &devicePath)
{
    emitDataChanged(devicePath, { CurrentPresetColumn, ManualPropertiesCountColumn, PresetsCountColumn });
}

void FSCameraLockPropertiesModel::init()
{
    d = new FSCameraLockPropertiesModelPrivate();

    retranslate();
    connect(fsTH, &FSTranslationsHelper::currentLanguageChanged,
            this, &FSCameraLockPropertiesModel::retranslate);

    setSupportRoles( { Qt::DisplayRole, Qt::ToolTipRole, Qt::TextAlignmentRole } );
}

void FSCameraLockPropertiesModel::retranslate()
{
    setHorizontalSectionNames( { tr("Name"), tr("Is connected"), tr("Current preset"), tr("Manual properties count"), tr("Presets count") }) ;
}
