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

#include <QAbstractItemModel>

#include <WebCamFS/Structs>

class FSCamerasStorage;

class FSAbstractCameraSettingsModelPrivate;

class FSAbstractCameraSettingsModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit FSAbstractCameraSettingsModel(QObject *parent = nullptr);
    ~FSAbstractCameraSettingsModel() override;

    void setCamerasStorage(FSCamerasStorage *camerasStorage);
    FSCamerasStorage *camerasStorage() const;

    std::vector<DevicePath> cachedVectorDevicePaths() const;

    int row(const DevicePath &devicePath);

    DevicePath devicePath(const QModelIndex &index)const;
    FSCamera *camera(const QModelIndex &index)const;

    QModelIndex index(int row,
                      int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index,
                 const QVariant &value,
                 int role = Qt::EditRole) override;

    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void updateDevices();

    virtual QVariant getDeviceData(const DevicePath &devicePath,
                                   int column,
                                   int role = Qt::DisplayRole) const = 0;

    virtual bool setDeviceData(const DevicePath &devicePath,
                               int column,
                               const QVariant &value,
                               int role = Qt::EditRole) const;

    void emitDataByRow(int row);
    void emitDataByRow(const DevicePath &devicePath);

    bool isSupportRole(int role) const;

protected:
    virtual std::vector<DevicePath> getDevicesFromStorage() const = 0;

    void setHorizontalSectionNames(const std::vector<QString> &vectorNames);
    std::vector<QString> vectorHorizontalSectionNames() const;

    void addDevice(const DevicePath &devicePath);
    void removeDevice(const DevicePath &devicePath);
    void removeDevice(int deviceIndex);

    void emitDataChanged(const DevicePath &devicePath, int column);
    void emitDataChanged(const DevicePath &devicePath, const std::vector<int> &columns);
    void emitDataChanged(int row, const std::vector<int> &columns);

    void setSupportRoles(const std::vector<int> &supportRoles);
    std::vector<int> supportRoles() const;

    virtual void connectByCamerasStorage(FSCamerasStorage *camerasStorage);
    virtual void disconnectByCamerasStorage(FSCamerasStorage *camerasStorage);

protected slots:
    virtual void addCamera(const DevicePath &devicePath);
    virtual void removeCamera(const DevicePath &devicePath);

private:
    FSAbstractCameraSettingsModelPrivate *d;

    void init();

    void updateCachedDevicePathsAndRows();
};
