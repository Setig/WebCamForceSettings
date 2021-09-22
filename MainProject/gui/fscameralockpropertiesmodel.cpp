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
            disconnect(d->lockPropertiesManager, SIGNAL(lockedCamera(DevicePath)),
                       this,                       SLOT(updateLockCameraColumns(DevicePath)));
            disconnect(d->lockPropertiesManager, SIGNAL(unlockedCamera(DevicePath)),
                       this,                       SLOT(updateUnlockCameraColumns(DevicePath)));

            disconnect(d->lockPropertiesManager, SIGNAL(lockedProperty(DevicePath,FSCameraProperty)),
                       this,                       SLOT(updateLockPropertyColumns(DevicePath,FSCameraProperty)));
            disconnect(d->lockPropertiesManager, SIGNAL(unlockedProperty(DevicePath,FSCameraProperty)),
                       this,                       SLOT(updateUnlockPropertyColumns(DevicePath,FSCameraProperty)));

            disconnect(d->lockPropertiesManager, SIGNAL(switchedToManualMode(DevicePath)),
                       this,                       SLOT(updateSwitchToManualModeColumns(DevicePath)));
            disconnect(d->lockPropertiesManager, SIGNAL(lockedPreset(DevicePath,QString)),
                       this,                       SLOT(updateLockPresetColumns(DevicePath,QString)));
            disconnect(d->lockPropertiesManager, SIGNAL(unlockedPreset(DevicePath)),
                       this,                       SLOT(updateUnlockPresetColumns(DevicePath)));
        }

        d->lockPropertiesManager = lockPropertiesManager;

        if (d->lockPropertiesManager) {
            connect(d->lockPropertiesManager, SIGNAL(lockedCamera(DevicePath)),
                    this,                       SLOT(updateLockCameraColumns(DevicePath)));
            connect(d->lockPropertiesManager, SIGNAL(unlockedCamera(DevicePath)),
                    this,                       SLOT(updateUnlockCameraColumns(DevicePath)));

            connect(d->lockPropertiesManager, SIGNAL(lockedProperty(DevicePath,FSCameraProperty)),
                    this,                       SLOT(updateLockPropertyColumns(DevicePath,FSCameraProperty)));
            connect(d->lockPropertiesManager, SIGNAL(unlockedProperty(DevicePath,FSCameraProperty)),
                    this,                       SLOT(updateUnlockPropertyColumns(DevicePath,FSCameraProperty)));

            connect(d->lockPropertiesManager, SIGNAL(switchedToManualMode(DevicePath)),
                    this,                       SLOT(updateSwitchToManualModeColumns(DevicePath)));
            connect(d->lockPropertiesManager, SIGNAL(lockedPreset(DevicePath,QString)),
                    this,                       SLOT(updateLockPresetColumns(DevicePath,QString)));
            connect(d->lockPropertiesManager, SIGNAL(unlockedPreset(DevicePath)),
                    this,                       SLOT(updateUnlockPresetColumns(DevicePath)));
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
        case 0: return Qt::AlignLeft;
        case 1: return Qt::AlignCenter;
        case 2: return Qt::AlignLeft;
        case 3: return Qt::AlignCenter;
        case 4: return Qt::AlignCenter;
        default:
            break;
        }

        return QVariant();
    }

    if (!d->lockPropertiesManager)
        return QVariant();

    switch (column) {
    case 0: return camerasStorage()->getCameraDisplayName(devicePath);
    case 1: return camerasStorage()->isCameraConnected(devicePath);
    case 2:
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
    case 3: return d->lockPropertiesManager->lockedProperties(devicePath).size();
    case 4: return camerasStorage()->getCameraUserPresetNames(devicePath).size();
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
        emitDataChanged(devicePath, { 0, 1 });
    }
}

void FSCameraLockPropertiesModel::removeCamera(const DevicePath &devicePath)
{
    if ( d->lockPropertiesManager &&
         !d->lockPropertiesManager->isLockedLeastOneProperty(devicePath) ) {
        removeDevice(devicePath);
    } else {
        emitDataChanged(devicePath, { 0, 1 });
    }
}

void FSCameraLockPropertiesModel::updateLockCameraColumns(const DevicePath &devicePath)
{
    emitDataChanged(devicePath, { 2, 3, 4 });
}

void FSCameraLockPropertiesModel::updateUnlockCameraColumns(const DevicePath &devicePath)
{
    emitDataChanged(devicePath, { 2, 3, 4 });
}

void FSCameraLockPropertiesModel::updateLockPropertyColumns(const DevicePath &devicePath,
                                                            FSCameraProperty property)
{
    Q_UNUSED(property);

    emitDataChanged(devicePath, { 2, 3, 4 });
}

void FSCameraLockPropertiesModel::updateUnlockPropertyColumns(const DevicePath &devicePath,
                                                              FSCameraProperty property)
{
    Q_UNUSED(property);

    emitDataChanged(devicePath, { 2, 3, 4 });
}

void FSCameraLockPropertiesModel::updateSwitchToManualModeColumns(const DevicePath &devicePath)
{
    emitDataChanged(devicePath, { 2, 3, 4 });
}

void FSCameraLockPropertiesModel::updateLockPresetColumns(const DevicePath &devicePath,
                                                          const QString &presetName)
{
    Q_UNUSED(presetName);

    emitDataChanged(devicePath, { 2, 3, 4 });
}

void FSCameraLockPropertiesModel::updateUnlockPresetColumns(const DevicePath &devicePath)
{
    emitDataChanged(devicePath, { 2, 3, 4 });
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
