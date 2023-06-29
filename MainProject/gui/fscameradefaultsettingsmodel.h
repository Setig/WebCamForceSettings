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

#include <WebCamFS/AbstractCameraSettingsModel>

class FSCameraDefaultSettingsModel : public FSAbstractCameraSettingsModel
{
    Q_OBJECT

public:
    explicit FSCameraDefaultSettingsModel(QObject *parent = nullptr);

    enum Column {
        NameColumn = 0,
        IsConnectedColumn,
        IsExistDefaultSettingsColumn,
        DefaultSettingsCountColumn
    };

    QVariant getDeviceData(const DevicePath &devicePath,
                           int column,
                           int role = Qt::DisplayRole) const override;

protected:
    std::vector<DevicePath> getDevicesFromStorage() const override;

    void connectByCamerasStorage(FSCamerasStorage *camerasStorage) override;
    void disconnectByCamerasStorage(FSCamerasStorage *camerasStorage) override;

protected slots:
    void addCamera(const DevicePath &devicePath) override;
    void removeCamera(const DevicePath &devicePath) override;

    void updateDefaultSettingsColumns(const DevicePath &devicePath);

private slots:
    void retranslate();
};
