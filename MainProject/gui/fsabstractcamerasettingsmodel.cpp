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

#include "fsabstractcamerasettingsmodel.h"

#include <WebCamFS/CamerasStorage>

#include <set>

typedef std::unordered_map<DevicePath, int> DevicePathsAndRowsUMap;

class FSAbstractCameraSettingsModelPrivate
{
public:
    FSAbstractCameraSettingsModelPrivate();

    FSCamerasStorage       *camerasStorage;
    std::vector<QString>    vectorHorizontalSectionNames;
    std::set<int>           supportRoles;

    std::vector<DevicePath> cachedVectorDevicePaths; // Always sorted by device paths, duplicates excluded

    DevicePathsAndRowsUMap umapDevicePathsAndRows;
};

FSAbstractCameraSettingsModelPrivate::FSAbstractCameraSettingsModelPrivate()
    : camerasStorage(nullptr)
{

}

FSAbstractCameraSettingsModel::FSAbstractCameraSettingsModel(QObject *parent) : QAbstractItemModel(parent)
{
    init();
}

FSAbstractCameraSettingsModel::~FSAbstractCameraSettingsModel()
{
    delete d;
    d = nullptr;
}

void FSAbstractCameraSettingsModel::setCamerasStorage(FSCamerasStorage *camerasStorage)
{
    if (d->camerasStorage != camerasStorage) {
        if (d->camerasStorage) {
            disconnect(d->camerasStorage, &FSCamerasStorage::addedCamera,
                       this,              &FSAbstractCameraSettingsModel::addedCamera);
            disconnect(d->camerasStorage, &FSCamerasStorage::removedCamera,
                       this,              &FSAbstractCameraSettingsModel::removedCamera);
        }

        d->camerasStorage = camerasStorage;

        if (d->camerasStorage) {
            connect(d->camerasStorage, &FSCamerasStorage::addedCamera,
                    this,              &FSAbstractCameraSettingsModel::addedCamera);
            connect(d->camerasStorage, &FSCamerasStorage::removedCamera,
                    this,              &FSAbstractCameraSettingsModel::removedCamera);
        }

        updateDevices();
    }
}

FSCamerasStorage *FSAbstractCameraSettingsModel::camerasStorage() const
{
    return d->camerasStorage;
}

std::vector<DevicePath> FSAbstractCameraSettingsModel::cachedVectorDevicePaths() const
{
    return d->cachedVectorDevicePaths;
}

int FSAbstractCameraSettingsModel::row(const DevicePath &devicePath)
{
    DevicePathsAndRowsUMap::const_iterator iterator = d->umapDevicePathsAndRows.find(devicePath);
    if (iterator != d->umapDevicePathsAndRows.end())
        return iterator->second;

    return -1;
}

DevicePath FSAbstractCameraSettingsModel::devicePath(const QModelIndex &index) const
{
    if ( index.isValid() &&
         index.model() == this &&
         index.column() >= 0 &&
         index.column() < int(d->vectorHorizontalSectionNames.size()) &&
         index.row() >= 0 &&
         index.row() < rowCount() ) {
        return d->cachedVectorDevicePaths.at(index.row());
    }

    return DevicePath();
}

FSCamera *FSAbstractCameraSettingsModel::camera(const QModelIndex &index) const
{
    if (!d->camerasStorage || index.model() != this)
        return nullptr;

    const DevicePath &devicePath = this->devicePath(index);

    if (devicePath.isEmpty())
        return nullptr;

    return d->camerasStorage->findCameraByDevicePath(devicePath);
}

QModelIndex FSAbstractCameraSettingsModel::index(int row,
                                                 int column,
                                                 const QModelIndex &parent) const
{
    if ( !parent.isValid() &&
         column >= 0 &&
         column < int(d->vectorHorizontalSectionNames.size()) &&
         row >= 0 &&
         row < rowCount() ) {
        return createIndex(row, column);
    }

    return QModelIndex();
}

QModelIndex FSAbstractCameraSettingsModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child);

    return QModelIndex();
}

int FSAbstractCameraSettingsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return int(d->cachedVectorDevicePaths.size());
}

int FSAbstractCameraSettingsModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return int(d->vectorHorizontalSectionNames.size());
}

QVariant FSAbstractCameraSettingsModel::data(const QModelIndex &index,
                                             int role) const
{
    if (!index.isValid())
        return QVariant();

    if (!d->camerasStorage || !isSupportRole(role))
        return QVariant();

    if (index.row() >= 0 && index.row() < int(d->cachedVectorDevicePaths.size())) {
        const DevicePath &devicePath = d->cachedVectorDevicePaths.at(index.row());
        return getDeviceData(devicePath, index.column(), role);
    }

    return QVariant();
}

bool FSAbstractCameraSettingsModel::setData(const QModelIndex &index,
                                            const QVariant &value,
                                            int role)
{
    if (!index.isValid() || !d->camerasStorage || !isSupportRole(role))
        return false;

    if (index.row() >= 0 && index.row() < int(d->cachedVectorDevicePaths.size())) {
        const DevicePath &devicePath = d->cachedVectorDevicePaths.at(index.row());
        return setDeviceData(devicePath, index.column(), value, role);
    }

    return false;
}

QVariant FSAbstractCameraSettingsModel::headerData(int section,
                                                   Qt::Orientation orientation,
                                                   int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        if ( section > 0 &&
             section < int(d->vectorHorizontalSectionNames.size()) ) {
            return d->vectorHorizontalSectionNames.at(section);
        }

        return QVariant();
    }

    return QAbstractItemModel::headerData(section, orientation, role);
}

Qt::ItemFlags FSAbstractCameraSettingsModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags result = QAbstractItemModel::flags(index);
    result |= Qt::ItemNeverHasChildren;
    return result;
}

void FSAbstractCameraSettingsModel::updateDevices()
{
    beginResetModel();
    d->cachedVectorDevicePaths = getDevicesFromStorage();
    std::sort(d->cachedVectorDevicePaths.begin(),
              d->cachedVectorDevicePaths.end());
    updateCachedDevicePathsAndRows();
    endResetModel();
}

bool FSAbstractCameraSettingsModel::setDeviceData(const DevicePath &devicePath,
                                                  int column,
                                                  const QVariant &value,
                                                  int role) const
{
    Q_UNUSED(devicePath);
    Q_UNUSED(column);
    Q_UNUSED(value);
    Q_UNUSED(role);

    return false;
}

void FSAbstractCameraSettingsModel::updateRow(const DevicePath &devicePath)
{
    std::vector<int> columns;

    const int columnCount = this->columnCount();

    columns.reserve(columnCount);

    for (int i = 0; i < columnCount; i++)
        columns.push_back(i);

    emitDataChanged(devicePath, columns);
}

bool FSAbstractCameraSettingsModel::isSupportRole(int role) const
{
    return (d->supportRoles.find(role) != d->supportRoles.end());
}

void FSAbstractCameraSettingsModel::setHorizontalSectionNames(const std::vector<QString> &vectorNames)
{
    beginResetModel();
    d->vectorHorizontalSectionNames = vectorNames;
    endResetModel();
}

std::vector<QString> FSAbstractCameraSettingsModel::vectorHorizontalSectionNames() const
{
    return d->vectorHorizontalSectionNames;
}

void FSAbstractCameraSettingsModel::addDevice(const DevicePath &devicePath)
{
    int beforeIndex = -1;

    const int count = int(d->cachedVectorDevicePaths.size());
    for (int i = 0; i < count; i++) {
        const QString &tmpDevicePath = d->cachedVectorDevicePaths.at(i);
        if (tmpDevicePath == devicePath) {
            return;
        }

        if (beforeIndex == -1 && tmpDevicePath < devicePath) {
            beforeIndex = i;
        }
    }

    if (beforeIndex == -1)
        beforeIndex = 0;
    else
        beforeIndex++;

    beginInsertRows(QModelIndex(), beforeIndex, beforeIndex);
    d->cachedVectorDevicePaths.insert(d->cachedVectorDevicePaths.begin() + beforeIndex,
                                      devicePath);
    updateCachedDevicePathsAndRows();
    endInsertRows();
}

void FSAbstractCameraSettingsModel::removeDevice(const DevicePath &devicePath)
{
    const int count = int(d->cachedVectorDevicePaths.size());
    for (int i = 0; i < count; i++) {
        const QString &tmpDevicePath = d->cachedVectorDevicePaths.at(i);
        if (tmpDevicePath == devicePath) {
            removeDevice(i);
            break;
        }
    }
}

void FSAbstractCameraSettingsModel::removeDevice(int deviceIndex)
{
    if (deviceIndex < 0 || deviceIndex >= int(d->cachedVectorDevicePaths.size()))
        return;

    beginRemoveRows(QModelIndex(), deviceIndex, deviceIndex);
    d->umapDevicePathsAndRows.erase(d->cachedVectorDevicePaths[deviceIndex]);
    updateCachedDevicePathsAndRows();
    endRemoveRows();
}

void FSAbstractCameraSettingsModel::emitDataChanged(const DevicePath &devicePath,
                                                    int column)
{
    const int row = this->row(devicePath);
    if (row != -1) {
        const QModelIndex index = this->index(row, column);
        emit dataChanged(index, index);
    }
}

void FSAbstractCameraSettingsModel::emitDataChanged(const DevicePath &devicePath,
                                                    std::vector<int> columns)
{
    emitDataChanged(row(devicePath), columns);
}

void FSAbstractCameraSettingsModel::emitDataChanged(int row,
                                                    std::vector<int> columns)
{
    if (row != -1) {
        for (int column : columns) {
            const QModelIndex index = this->index(row, column);
            emit dataChanged(index, index);
        }
    }
}

void FSAbstractCameraSettingsModel::setSupportRoles(std::vector<int> supportRoles)
{
    d->supportRoles.clear();

    for (int role : supportRoles) {
        d->supportRoles.insert(role);
    }
}

std::vector<int> FSAbstractCameraSettingsModel::supportRoles() const
{
    std::vector<int> result;
    result.reserve(d->supportRoles.size());

    for (int role : d->supportRoles) {
        result.push_back(role);
    }

    return result;
}

void FSAbstractCameraSettingsModel::addedCamera(const DevicePath &devicePath)
{
    Q_UNUSED(devicePath);

    // do nothing
}

void FSAbstractCameraSettingsModel::removedCamera(const DevicePath &devicePath)
{
    Q_UNUSED(devicePath);

    // do nothing
}

void FSAbstractCameraSettingsModel::init()
{
    d = new FSAbstractCameraSettingsModelPrivate();
}

void FSAbstractCameraSettingsModel::updateCachedDevicePathsAndRows()
{
    const int count = int(d->cachedVectorDevicePaths.size());

    d->umapDevicePathsAndRows.clear();
    d->umapDevicePathsAndRows.reserve(count);

    for (int i = 0; i < count; i++) {
        d->umapDevicePathsAndRows.insert( { d->cachedVectorDevicePaths.at(i), i } );
    }
}
