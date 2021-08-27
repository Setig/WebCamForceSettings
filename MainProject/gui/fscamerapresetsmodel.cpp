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

#include "fscamerapresetsmodel.h"

#include <WebCamFS/CamerasStorage>
#include <WebCamFS/TranslationsHelper>

FSCameraPresetsModel::FSCameraPresetsModel(QObject *parent)
    : FSAbstractCameraSettingsModel(parent)
{
    retranslate();
    connect(fsTH, &FSTranslationsHelper::currentLanguageChanged,
            this, &FSCameraPresetsModel::retranslate);

    setSupportRoles( { Qt::DisplayRole, Qt::ToolTipRole, Qt::TextAlignmentRole } );
}

QVariant FSCameraPresetsModel::getDeviceData(const DevicePath &devicePath,
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
    case 1: return (camerasStorage()->findCameraByDevicePath(devicePath) != nullptr);
    case 2: return camerasStorage()->isCameraUserPresetsUsed(devicePath);
    case 3: return camerasStorage()->getCameraUserPresets(devicePath).size();
    default:
        break;
    }

    return QVariant();
}

std::vector<DevicePath> FSCameraPresetsModel::getDevicesFromStorage() const
{
    std::vector<DevicePath> result;

    FSCamerasStorage *camerasStorage = this->camerasStorage();
    if (camerasStorage) {
        const FSCameraPathUserPresetsUMap umapCameraPathUserPresets = camerasStorage->getCameraUserPresets();
        const std::vector<DevicePath> vecAvailableDevicePaths = camerasStorage->availableDevicePaths();

        result.reserve(umapCameraPathUserPresets.size() + vecAvailableDevicePaths.size());

        for (const auto &[devicePath, preset] : umapCameraPathUserPresets) {
            result.push_back(devicePath);
        }

        for (const DevicePath &devicePath : vecAvailableDevicePaths) {
            if ( !camerasStorage->isContaintsBlackList(devicePath) &&
                 umapCameraPathUserPresets.find(devicePath) == umapCameraPathUserPresets.end()) {
                result.push_back(devicePath);
            }
        }
    }

    return result;
}

void FSCameraPresetsModel::addedCamera(const DevicePath &devicePath)
{
    FSCamerasStorage *camerasStorage = this->camerasStorage();
    if ( camerasStorage &&
         !camerasStorage->isCameraUserPresetsUsed(devicePath) ) {
        addDevice(devicePath);
    } else {
        emitDataChanged(devicePath, { 0, 1 });
    }
}

void FSCameraPresetsModel::removedCamera(const DevicePath &devicePath)
{
    FSCamerasStorage *camerasStorage = this->camerasStorage();
    if ( camerasStorage &&
         !camerasStorage->isCameraUserPresetsUsed(devicePath) ) {
        removeDevice(devicePath);
    } else {
        emitDataChanged(devicePath, { 0, 1 });
    }
}

void FSCameraPresetsModel::retranslate()
{
    setHorizontalSectionNames( { tr("Name"), tr("Is connected"), tr("Is exist presets"), tr("Presets count") } );
}
