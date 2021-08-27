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

#include <vector>
#include <unordered_map>

#include <WebCamFS/Structs>

typedef std::unordered_map<DevicePath, FSCameraData> FSCamerasDataUMap;

class FSCoreCamerasStorage;

class FSCameraPrivate;

class FS_LIB_EXPORT FSCamera
{
public:
    explicit FSCamera(const FSCameraData &cameraData);
    virtual ~FSCamera();

    void setCoreCamerasStorage(FSCoreCamerasStorage *coreCamerasStorage);
    FSCoreCamerasStorage *coreCamerasStorage() const;

    static bool initializeWinCOMLibrary();
    static void uninitializeWinCOMLibrary();

    static void printCamerasInfo();
    static FSCamera *findCameraByDevicePath(const DevicePath &devicePath);
    static FSCamerasDataUMap availableCamerasDataUMap();

    static void releaseCameraData(FSCameraData &cameraData);

    DeviceName name() const;
    DevicePath devicePath() const;
    DeviceName userName() const;
    DeviceName displayName() const;

    bool isBlackListed() const;

    bool getRange(FSCameraProperty property, FSRangeParams &rangeParam) const;
    bool get(FSCameraProperty property, FSValueParams &valueParams) const;
    bool set(FSCameraProperty property, const FSValueParams &valueParams);

private:
    FSCameraPrivate *d;

    void init(const FSCameraData &cameraData);
};
