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

#include "fscameradefaultsettingsmodel.h"

#include <WebCamFS/CamerasStorage>
#include <WebCamFS/TranslationsHelper>

FSCameraDefaultSettingsModel::FSCameraDefaultSettingsModel(QObject *parent)
    : FSAbstractCameraSettingsModel(parent)
{
    retranslate();
    connect(fsTH, &FSTranslationsHelper::currentLanguageChanged,
            this, &FSCameraDefaultSettingsModel::retranslate);

    setSupportRoles( { Qt::DisplayRole, Qt::ToolTipRole, Qt::TextAlignmentRole } );
}

QVariant FSCameraDefaultSettingsModel::getDeviceData(const DevicePath &devicePath,
                                                     int column,
                                                     int role) const
{
    if (role == Qt::TextAlignmentRole) {
        switch (column) {
        case 0: return Qt::AlignLeft;
        case 1: return Qt::AlignCenter;
        case 2: return Qt::AlignCenter;
        case 3: return Qt::AlignLeft;
        default:
            break;
        }

        return QVariant();
    }

    switch (column) {
    case 0: return camerasStorage()->getCameraDisplayName(devicePath);
    case 1: return camerasStorage()->isCameraConnected(devicePath);
    case 2: return camerasStorage()->isUserDefaultValuesUsed(devicePath);
    case 3: return camerasStorage()->getUserDefaultValues(devicePath).size();
    default:
        break;
    }

    return QVariant();
}

std::vector<DevicePath> FSCameraDefaultSettingsModel::getDevicesFromStorage() const
{
    std::vector<DevicePath> result;

    FSCamerasStorage *camerasStorage = this->camerasStorage();
    if (camerasStorage) {
        const FSCameraPathValuesUMap umapCameraPathValues     = camerasStorage->getUserDefaultValues();
        const std::vector<DevicePath> vecAvailableDevicePaths = camerasStorage->availableDevicePaths();

        result.reserve(umapCameraPathValues.size() + vecAvailableDevicePaths.size());

        for (const auto &[devicePath, propertiesValue] : umapCameraPathValues) {
            result.push_back(devicePath);
        }

        for (const DevicePath &devicePath : vecAvailableDevicePaths) {
            if ( !camerasStorage->isContaintsBlackList(devicePath) &&
                 umapCameraPathValues.find(devicePath) == umapCameraPathValues.end() ) {
                result.push_back(devicePath);
            }
        }
    }

    return result;
}

void FSCameraDefaultSettingsModel::connectByCamerasStorage(FSCamerasStorage *camerasStorage)
{
    if (camerasStorage) {
        connect(camerasStorage, SIGNAL(userDefaultValuesChanged(DevicePath)),
                this,             SLOT(updateDefaultSettingsColumns(DevicePath)));
    }

    FSAbstractCameraSettingsModel::connectByCamerasStorage(camerasStorage);
}

void FSCameraDefaultSettingsModel::disconnectByCamerasStorage(FSCamerasStorage *camerasStorage)
{
    if (camerasStorage) {
        disconnect(camerasStorage, SIGNAL(userDefaultValuesChanged(DevicePath)),
                   this,             SLOT(updateDefaultSettingsColumns(DevicePath)));
    }

    FSAbstractCameraSettingsModel::disconnectByCamerasStorage(camerasStorage);
}

void FSCameraDefaultSettingsModel::addCamera(const DevicePath &devicePath)
{
    FSCamerasStorage *camerasStorage = this->camerasStorage();
    if ( camerasStorage &&
         !camerasStorage->isUserDefaultValuesUsed(devicePath) ) {
        addDevice(devicePath);
    } else {
        emitDataChanged(devicePath, { 0, 1 });
    }
}

void FSCameraDefaultSettingsModel::removeCamera(const DevicePath &devicePath)
{
    FSCamerasStorage *camerasStorage = this->camerasStorage();
    if ( camerasStorage &&
         !camerasStorage->isUserDefaultValuesUsed(devicePath) ) {
        removeDevice(devicePath);
    } else {
        emitDataChanged(devicePath, { 0, 1 });
    }
}

void FSCameraDefaultSettingsModel::updateDefaultSettingsColumns(const DevicePath &devicePath)
{
    emitDataChanged(devicePath, { 2, 3 });
}

void FSCameraDefaultSettingsModel::retranslate()
{
    setHorizontalSectionNames( { tr("Name"), tr("Is connected"), tr("Is exist default settings"), tr("Default settings count") }) ;
}
