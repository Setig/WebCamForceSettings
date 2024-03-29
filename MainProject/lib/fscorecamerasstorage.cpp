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

#include "fscorecamerasstorage.h"

#include <unordered_set>

#include <QMutexLocker>

#include <WebCamFS/Settings>

#define MAX_FREE_CAM_NUMBER 1000

typedef std::unordered_set<DevicePath> FSDevicePathsUSet;

class FSCoreCamerasStoragePrivate
{
public:
    FSCoreCamerasStoragePrivate();

    QRecursiveMutex cameraMutex;

    FSCameraPathsUMap umapCameraPaths;
    FSCameraNamesUMap umapCameraNames;

    FSCameraPathNamesUMap umapCameraPathUserNames;
    FSCameraNamePathsUMap umapCameraUserNamePaths;
    FSDevicePathsUSet     usetBlackListDevices;
};

FSCoreCamerasStoragePrivate::FSCoreCamerasStoragePrivate()
{
    const size_t standartCamerasCount = FSSettings::usuallyAvailableCamerasCount();

    umapCameraPaths.reserve(standartCamerasCount);
    umapCameraNames.reserve(standartCamerasCount);

    umapCameraPathUserNames.reserve(standartCamerasCount);
    umapCameraUserNamePaths.reserve(standartCamerasCount);
    usetBlackListDevices.reserve(standartCamerasCount);
}

FSCoreCamerasStorage::FSCoreCamerasStorage(QObject *parent)
    : QObject(parent)
{
    init();
}

FSCoreCamerasStorage::~FSCoreCamerasStorage()
{
    // Update statistics param
    FSSettings::setUsuallyAvailableCamerasCount(int(d->umapCameraPaths.size()));

    for (const auto &[devicePath, camera] : qAsConst(d->umapCameraPaths))
        delete camera;

    delete d;
    d = nullptr;
}

DeviceName FSCoreCamerasStorage::getFreeDeviceName(const DeviceName &deviceName) const
{
    // Check free device name
    if (d->umapCameraNames.find(deviceName) == d->umapCameraNames.end())
        return deviceName;

    // Check free device name with number
    for (int i = 2; i < MAX_FREE_CAM_NUMBER; i++) {
        const DeviceName newDeviceName = QStringLiteral("%1 (%2)").arg(deviceName, QString::number(i));

        if (d->umapCameraNames.find(newDeviceName) == d->umapCameraNames.end())
            return deviceName;
    }

    return deviceName;
}

std::vector<DevicePath> FSCoreCamerasStorage::availableDevicePaths() const
{
    QMutexLocker(&d->cameraMutex);

    std::vector<DevicePath> result;
    result.reserve(d->umapCameraPaths.size());

    for (const auto &[devicePath, camera] : qAsConst(d->umapCameraPaths))
        result.push_back(devicePath);

    return result;
}

std::vector<FSCamera *> FSCoreCamerasStorage::availableCameras() const
{
    QMutexLocker(&d->cameraMutex);

    std::vector<FSCamera *> result;
    result.reserve(d->umapCameraPaths.size());

    for (const auto &[devicePath, camera] : qAsConst(d->umapCameraPaths))
        result.push_back(camera);

    return result;
}

DeviceName FSCoreCamerasStorage::getCameraName(const DevicePath &devicePath) const
{
    QMutexLocker(&d->cameraMutex);

    FSCameraPathsUMap::const_iterator iterator = d->umapCameraPaths.find(devicePath);
    if (iterator != d->umapCameraPaths.cend())
        return iterator->second->name();

    return DevicePath();
}

DeviceName FSCoreCamerasStorage::getCameraDisplayName(const DevicePath &devicePath) const
{
    if (isCameraUserNameUsed(devicePath))
        return getCameraUserName(devicePath);

    QMutexLocker(&d->cameraMutex);

    FSCameraPathsUMap::const_iterator iterator = d->umapCameraPaths.find(devicePath);
    if (iterator != d->umapCameraPaths.cend())
        return iterator->second->name();

    return devicePath;
}

void FSCoreCamerasStorage::registerCamera(FSCamera *camera)
{
    QMutexLocker(&d->cameraMutex);

    const DevicePath &devicePath = camera->devicePath();

    if (d->umapCameraPaths.find(devicePath) != d->umapCameraPaths.end())
        return;

    d->umapCameraPaths.insert( { devicePath, camera } );
    d->umapCameraNames.insert( { camera->name(), camera } );

    camera->setCamerasStorage(this);

    emit addedCamera(devicePath);
}

void FSCoreCamerasStorage::unregisterCamera(FSCamera *camera)
{
    QMutexLocker(&d->cameraMutex);

    const DevicePath &devicePath = camera->devicePath();

    FSCameraPathsUMap::const_iterator iterator = d->umapCameraPaths.find(devicePath);

    if (iterator == d->umapCameraPaths.cend())
        return;

    d->umapCameraPaths.erase(iterator);
    d->umapCameraNames.erase(camera->name());

    camera->setCamerasStorage(nullptr);

    emit removedCamera(devicePath);
}

bool FSCoreCamerasStorage::isCameraConnected(const DevicePath &devicePath) const
{
    QMutexLocker(&d->cameraMutex);

    return (d->umapCameraPaths.find(devicePath) != d->umapCameraPaths.end());
}

QRecursiveMutex *FSCoreCamerasStorage::cameraRecursiveMutex() const
{
    return &d->cameraMutex;
}

FSCamera *FSCoreCamerasStorage::findCameraByDevicePath(const DevicePath &devicePath) const
{
    QMutexLocker(&d->cameraMutex);

    FSCameraPathsUMap::const_iterator iterator = d->umapCameraPaths.find(devicePath);

    if (iterator == d->umapCameraPaths.cend())
        return nullptr;
    else
        return iterator->second;
}

FSCameraPathsUMap FSCoreCamerasStorage::getCameraPathUMap() const
{
    QMutexLocker(&d->cameraMutex);

    return d->umapCameraPaths;
}

DevicePath FSCoreCamerasStorage::findCameraPathByUserName(const DeviceName &userName)
{
    if (!userName.isEmpty()) {
        FSCameraNamePathsUMap::const_iterator iterator = d->umapCameraUserNamePaths.find(userName);
        if (iterator != d->umapCameraUserNamePaths.cend()) {
            return iterator->second;
        }
    }

    return DevicePath();
}

void FSCoreCamerasStorage::setCameraUserNames(const FSCameraPathNamesUMap &umapCameraUserNames)
{
    FSCameraPathNamesUMap filteredUMapCameraUserNames;

    for (const auto &[devicePath, userName] : umapCameraUserNames) {
        if (!devicePath.isEmpty() && !userName.isEmpty()) {
            filteredUMapCameraUserNames.insert_or_assign(devicePath, userName);
        }
    }

    const FSCameraPathNamesUMap oldUMapCameraUserNames = d->umapCameraPathUserNames;
    d->umapCameraPathUserNames = filteredUMapCameraUserNames;

    d->umapCameraUserNamePaths.clear();
    for (const auto &[devicePath, userName] : qAsConst(d->umapCameraPathUserNames)) {
        d->umapCameraUserNamePaths.insert( { userName, devicePath } );
    }

    for (const auto &[devicePath, userName] : oldUMapCameraUserNames) {
        FSCameraPathNamesUMap::const_iterator iterator = d->umapCameraPathUserNames.find(devicePath);

        if (iterator != d->umapCameraPathUserNames.cend()) {
            const QString oldUserName = userName;

            if (oldUserName != iterator->first) {
                emit cameraUserNameChanged(devicePath);
            }
        } else {
            emit cameraUserNameChanged(devicePath);
        }
    }

    for (const auto &[devicePath, userName] : qAsConst(d->umapCameraPathUserNames)) {
        FSCameraPathNamesUMap::const_iterator iterator = oldUMapCameraUserNames.find(devicePath);

        if (iterator == oldUMapCameraUserNames.cend()) {
            emit cameraUserNameChanged(devicePath);
        }
    }
}

bool FSCoreCamerasStorage::setCameraUserName(const DevicePath &devicePath, const DeviceName &userName)
{
    if (devicePath.isEmpty())
        return false;

    if (userName.isEmpty()) {
        cameraUserNameRemove(devicePath);
        return true;
    }

    // Check user name is not duplicated
    if (!findCameraPathByUserName(userName).isEmpty())
        return false;

    FSCameraPathNamesUMap::const_iterator iterator = d->umapCameraPathUserNames.find(devicePath);

    if (iterator == d->umapCameraPathUserNames.cend()) {
        d->umapCameraPathUserNames.insert( { devicePath, userName } );
        d->umapCameraUserNamePaths.insert( { userName, devicePath } );

        emit cameraUserNameChanged(devicePath);
    } else {
        const QString oldUserName = iterator->second;

        if (oldUserName != userName) {
            d->umapCameraPathUserNames.insert_or_assign(devicePath, userName);
            d->umapCameraUserNamePaths.erase(oldUserName);
            d->umapCameraUserNamePaths.insert( { userName, devicePath } );

            emit cameraUserNameChanged(devicePath);
        }
    }

    return true;
}

bool FSCoreCamerasStorage::isCameraUserNameUsed(const DevicePath &devicePath) const
{
    return (d->umapCameraPathUserNames.find(devicePath) != d->umapCameraPathUserNames.end());
}

FSCameraPathNamesUMap FSCoreCamerasStorage::getCameraUserNames()
{
    return d->umapCameraPathUserNames;
}

DeviceName FSCoreCamerasStorage::getCameraUserName(const DevicePath &devicePath) const
{
    FSCameraPathNamesUMap::const_iterator iterator = d->umapCameraPathUserNames.find(devicePath);
    if (iterator != d->umapCameraPathUserNames.cend())
        return iterator->second;

    return DeviceName();
}

void FSCoreCamerasStorage::cameraUserNameRemove(const DevicePath &devicePath)
{
    FSCameraPathNamesUMap::const_iterator iterator = d->umapCameraPathUserNames.find(devicePath);

    if (iterator != d->umapCameraPathUserNames.cend()) {
        d->umapCameraPathUserNames.erase(iterator);
        d->umapCameraUserNamePaths.erase(iterator->second);

        emit cameraUserNameChanged(devicePath);
    }
}

void FSCoreCamerasStorage::cameraUserNameClear()
{
    const FSCameraPathNamesUMap oldUMapCameraUserNames = d->umapCameraPathUserNames;

    d->umapCameraPathUserNames.clear();
    d->umapCameraUserNamePaths.clear();

    for (const auto &[devicePath, name] : oldUMapCameraUserNames) {
        emit cameraUserNameChanged(devicePath);
    }
}

void FSCoreCamerasStorage::saveUserNamesSettings()
{
    QVariantMap varMap;

    for (const auto &[devicePath, userName] : qAsConst(d->umapCameraPathUserNames)) {
        if (!devicePath.isEmpty() && !userName.isEmpty()) {
            varMap.insert(devicePath, userName);
        }
    }

    FSSettings::setUserNames(varMap);
}

void FSCoreCamerasStorage::loadUserNamesSettings()
{
    const QVariantMap varMap = FSSettings::userNames();
    FSCameraPathNamesUMap umapCameraUserNames;

    for (QVariantMap::const_iterator iterator = varMap.begin();
         iterator != varMap.cend();
         ++iterator) {
        const DevicePath &devicePath = iterator.key();
        if (!devicePath.isEmpty()) {
            const DeviceName &userName = iterator.value().toString();

            if (!userName.isEmpty()) {
                umapCameraUserNames.insert_or_assign(devicePath, userName);
            }
        }
    }

    setCameraUserNames(umapCameraUserNames);
}

void FSCoreCamerasStorage::setBlackList(const std::vector<DevicePath> &devicePaths)
{
    const FSDevicePathsUSet oldUSetBlackListDevices = d->usetBlackListDevices;
    d->usetBlackListDevices.clear();

    for (const DevicePath &devicePath: devicePaths) {
        insertBlackList(devicePath);
    }

    // Find new
    for (const DevicePath &devicePath : qAsConst(d->usetBlackListDevices)) {
        if (oldUSetBlackListDevices.find(devicePath) == oldUSetBlackListDevices.cend())
            emit addedBlacklistCamera(devicePath);
    }

    // Find removed
    for (const DevicePath &devicePath : oldUSetBlackListDevices) {
        if (d->usetBlackListDevices.find(devicePath) == d->usetBlackListDevices.end())
            emit removedBlacklistCamera(devicePath);
    }
}

void FSCoreCamerasStorage::insertBlackList(const DevicePath &devicePath)
{
    if (d->usetBlackListDevices.find(devicePath) != d->usetBlackListDevices.end())
        return;

    d->usetBlackListDevices.insert(devicePath);

    emit addedBlacklistCamera(devicePath);
}

void FSCoreCamerasStorage::removeBlackList(const DevicePath &devicePath)
{
    FSDevicePathsUSet::const_iterator iterator = d->usetBlackListDevices.find(devicePath);

    if (iterator == d->usetBlackListDevices.cend())
        return;

    d->usetBlackListDevices.erase(iterator);

    emit removedBlacklistCamera(devicePath);
}

void FSCoreCamerasStorage::clearBlackList()
{
    const FSDevicePathsUSet oldUSetBlackListDevices = d->usetBlackListDevices;
    d->usetBlackListDevices.clear();

    for (const DevicePath &devicePath : oldUSetBlackListDevices) {
        emit removedBlacklistCamera(devicePath);
    }
}

bool FSCoreCamerasStorage::isContaintsBlackList(const DevicePath &devicePath) const
{
    return (d->usetBlackListDevices.find(devicePath) != d->usetBlackListDevices.end());
}

void FSCoreCamerasStorage::saveBlackList()
{
    QVariantList varList;

    for (const DevicePath &devicePath : qAsConst(d->usetBlackListDevices)) {
        if (!devicePath.isEmpty()) {
            varList.push_back(devicePath);
        }
    }

    FSSettings::setBlackList(varList);
}

void FSCoreCamerasStorage::loadBlackList()
{
    const QVariantList varList = FSSettings::blackList();
    std::vector<DevicePath> devicePaths;

    for (const QVariant &varDevicePath : varList) {
        if (!varDevicePath.isNull()) {
            const DevicePath devicePath = varDevicePath.toString();

            if (!devicePath.isEmpty()) {
                devicePaths.push_back(devicePath);
            }
        }
    }

    setBlackList(devicePaths);
}

void FSCoreCamerasStorage::saveAll()
{
    saveUserNamesSettings();
    saveBlackList();

    FSCoreSettings::sync();
}

void FSCoreCamerasStorage::loadAll()
{
    loadUserNamesSettings();
    loadBlackList();
}

void FSCoreCamerasStorage::init()
{
    d = new FSCoreCamerasStoragePrivate();
}
