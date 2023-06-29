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

#include "fscamerausersettingsmodel.h"

#include <WebCamFS/CamerasStorage>
#include <WebCamFS/TranslationsHelper>

FSCameraUserSettingsModel::FSCameraUserSettingsModel(QObject *parent)
    : FSAbstractCameraSettingsModel(parent)
{
    retranslate();
    connect(fsTH, &FSTranslationsHelper::currentLanguageChanged,
            this, &FSCameraUserSettingsModel::retranslate);

    setSupportRoles( { Qt::DisplayRole, Qt::ToolTipRole, Qt::TextAlignmentRole, Qt::EditRole } );
}

Qt::ItemFlags FSCameraUserSettingsModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags result = FSAbstractCameraSettingsModel::flags(index);

    if ( index.isValid() &&
         (index.column() == BlacklistedColumn || index.column() == UserNameColumn) &&
         camerasStorage() &&
         index.row() >= 0 &&
         index.row() < rowCount() ) {
        result |= Qt::ItemIsEditable;
    }

    return result;
}

QVariant FSCameraUserSettingsModel::getDeviceData(const DevicePath &devicePath,
                                                  int column,
                                                  int role) const
{    
    if (role == Qt::TextAlignmentRole) {
        switch (column) {
        case PathColumn:
            return Qt::AlignLeft;
        case NameColumn:
            return Qt::AlignLeft;
        case IsConnectedColumn:
            return Qt::AlignCenter;
        case BlacklistedColumn:
            return Qt::AlignCenter;
        case UserNameColumn:
            return Qt::AlignLeft;
        default:
            break;
        }

        return QVariant();
    }

    if ( role == Qt::EditRole &&
         column != BlacklistedColumn &&
         column != UserNameColumn )
        return QVariant();

    switch (column) {
    case PathColumn:
        return devicePath;
    case NameColumn:
        return camerasStorage()->getCameraName(devicePath);
    case IsConnectedColumn:
        return camerasStorage()->isCameraConnected(devicePath);
    case BlacklistedColumn:
        return camerasStorage()->isContaintsBlackList(devicePath);
    case UserNameColumn:
        return camerasStorage()->getCameraUserName(devicePath);
    default:
        break;
    }

    return QVariant();
}

bool FSCameraUserSettingsModel::setDeviceData(const DevicePath &devicePath,
                                              int column,
                                              const QVariant &value,
                                              int role) const
{
    if (devicePath.isEmpty() || role != Qt::EditRole || (column != BlacklistedColumn && column != UserNameColumn))
        return false;

    if (column == UserNameColumn) {
        const DeviceName &newUserName = value.toString().trimmed();

        if (!newUserName.isEmpty())
            camerasStorage()->setCameraUserName(devicePath, value.toString());
        else
            camerasStorage()->cameraUserNameRemove(devicePath);
    } else {
        const bool isBlackListed = value.toBool();

        if (isBlackListed)
            camerasStorage()->insertBlackList(devicePath);
        else
            camerasStorage()->removeBlackList(devicePath);
    }

    return true;
}

std::vector<DevicePath> FSCameraUserSettingsModel::getDevicesFromStorage() const
{
    std::vector<DevicePath> result;

    FSCamerasStorage *camerasStorage = this->camerasStorage();
    if (camerasStorage) {
        const FSCameraPathNamesUMap umapCameraUserNames       = camerasStorage->getCameraUserNames();
        const std::vector<DevicePath> vecAvailableDevicePaths = camerasStorage->availableDevicePaths();

        result.reserve(umapCameraUserNames.size() + vecAvailableDevicePaths.size());

        for (const auto &[devicePath, deviceName] : umapCameraUserNames) {
            result.push_back(devicePath);
        }

        for (const DevicePath &devicePath : vecAvailableDevicePaths) {
            if (umapCameraUserNames.find(devicePath) == umapCameraUserNames.cend()) {
                result.push_back(devicePath);
            }
        }
    }

    return result;
}

void FSCameraUserSettingsModel::connectByCamerasStorage(FSCamerasStorage *camerasStorage)
{
    if (camerasStorage) {
        connect(camerasStorage, &FSCamerasStorage::cameraUserNameChanged,
                this,           &FSCameraUserSettingsModel::updateUserSettingsColumns);
        connect(camerasStorage, &FSCamerasStorage::addedBlacklistCamera,
                this,           &FSCameraUserSettingsModel::updateUserSettingsColumns);
        connect(camerasStorage, &FSCamerasStorage::removedBlacklistCamera,
                this,           &FSCameraUserSettingsModel::updateUserSettingsColumns);
    }

    FSAbstractCameraSettingsModel::connectByCamerasStorage(camerasStorage);
}

void FSCameraUserSettingsModel::disconnectByCamerasStorage(FSCamerasStorage *camerasStorage)
{
    if (camerasStorage) {
        disconnect(camerasStorage, &FSCamerasStorage::cameraUserNameChanged,
                   this,           &FSCameraUserSettingsModel::updateUserSettingsColumns);
        disconnect(camerasStorage, &FSCamerasStorage::addedBlacklistCamera,
                   this,           &FSCameraUserSettingsModel::updateUserSettingsColumns);
        disconnect(camerasStorage, &FSCamerasStorage::removedBlacklistCamera,
                   this,           &FSCameraUserSettingsModel::updateUserSettingsColumns);
    }

    FSAbstractCameraSettingsModel::disconnectByCamerasStorage(camerasStorage);
}

void FSCameraUserSettingsModel::addCamera(const DevicePath &devicePath)
{
    FSCamerasStorage *camerasStorage = this->camerasStorage();
    if ( camerasStorage &&
         !camerasStorage->isCameraUserNameUsed(devicePath) ) {
        addDevice(devicePath);
    } else {
        emitDataChanged(devicePath, { NameColumn, IsConnectedColumn });
    }
}

void FSCameraUserSettingsModel::removeCamera(const DevicePath &devicePath)
{
    FSCamerasStorage *camerasStorage = this->camerasStorage();
    if ( camerasStorage &&
         !camerasStorage->isCameraUserNameUsed(devicePath) ) {
        removeDevice(devicePath);
    } else {
        emitDataChanged(devicePath, { NameColumn, IsConnectedColumn });
    }
}

void FSCameraUserSettingsModel::updateUserSettingsColumns(const DevicePath &devicePath)
{
    emitDataChanged(devicePath, { BlacklistedColumn, UserNameColumn });
}

void FSCameraUserSettingsModel::retranslate()
{
    setHorizontalSectionNames( { tr("Path"), tr("Name"), tr("Is connected"), tr("Blacklisted"), tr("User name") } );
}
