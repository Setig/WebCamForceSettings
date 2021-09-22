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

#include <WebCamFS/Global>

#include <QObject>

#include <WebCamFS/Camera>
#include <WebCamFS/Structs>

#include <unordered_map>

class QRecursiveMutex;

typedef std::unordered_map<DevicePath, FSCamera *> FSCameraPathsUMap;
typedef std::unordered_map<DeviceName, FSCamera *> FSCameraNamesUMap;
typedef std::unordered_map<DevicePath, DeviceName> FSCameraPathNamesUMap;
typedef std::unordered_map<DeviceName, DevicePath> FSCameraNamePathsUMap;

class FSCoreCamerasStoragePrivate;

class FS_LIB_EXPORT FSCoreCamerasStorage : public QObject
{
    Q_OBJECT

public:
    explicit FSCoreCamerasStorage(QObject *parent = nullptr);
    ~FSCoreCamerasStorage() override;

    DeviceName getFreeDeviceName(const DeviceName &deviceName) const;

    std::vector<DevicePath> availableDevicePaths() const;
    std::vector<FSCamera *> availableCameras() const;

    DeviceName getCameraName(const DevicePath &devicePath) const;
    DeviceName getCameraDisplayName(const DevicePath &devicePath) const;

    void registerCamera(FSCamera *cameraData);
    void unregisterCamera(FSCamera *camera);

    bool isCameraConnected(const DevicePath &devicePath) const;

    QRecursiveMutex *cameraRecursiveMutex() const;

    FSCamera *findCameraByDevicePath(const DevicePath &devicePath) const;
    FSCameraPathsUMap getCameraPathUMap() const;

    // Camera user names
    DevicePath findCameraPathByUserName(const DeviceName &userName);

    void setCameraUserNames(const FSCameraPathNamesUMap &umapCameraUserNames);
    bool setCameraUserName(const DevicePath &devicePath, const DeviceName &userName);

    bool isCameraUserNameUsed(const DevicePath &devicePath) const;

    FSCameraPathNamesUMap getCameraUserNames();
    DeviceName getCameraUserName(const DevicePath &devicePath) const;

    void cameraUserNameRemove(const DevicePath &devicePath);

    void cameraUserNameClear();

    void saveUserNamesSettings();
    void loadUserNamesSettings();


    // Camera blacklist
    void setBlackList(const std::vector<DevicePath> &devicePaths);

    void insertBlackList(const DevicePath &devicePath);

    void removeBlackList(const DevicePath &devicePath);

    void clearBlackList();

    bool isContaintsBlackList(const DevicePath &devicePath) const;

    void saveBlackList();
    void loadBlackList();

public slots:
    virtual void saveAll();
    virtual void loadAll();

signals:
    void addedCamera(const DevicePath &devicePath);
    void removedCamera(const DevicePath &devicePath);

    void cameraUserNameChanged(const DevicePath &devicePath);

    void addedBlacklistCamera(const DevicePath &devicePath);
    void removedBlacklistCamera(const DevicePath &devicePath);

private:
    FSCoreCamerasStoragePrivate *d;

    void init();
};
