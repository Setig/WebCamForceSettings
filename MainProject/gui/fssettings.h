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

#include <WebCamFS/CoreSettings>

class FSSettings : public FSCoreSettings
{
public:
    static void initialization();
    static void uninitialization();

    static void setCurrentLocale(const QLocale &locale);
    static QLocale currentLocale();
    static QLocale defaultCurrentLocale();

    static void setCustomStyleIndex(int styleIndex);
    static int customStyleIndex();
    static int defaultCustomStyleIndex();

    static void setCameraDetectionEnable(bool isEnable);
    static bool isCameraDetectionEnable();
    static bool defaultIsCameraDetectionEnable();

    static void setCameraDetectionInterval(int msec);
    static int cameraDetectionInterval();
    static int defaultCameraDetectionInterval();

    static void setCameraValueUpdateEnable(bool isEnable);
    static bool isCameraValueUpdateEnable();
    static bool defaultIsCameraValueUpdateEnable();

    static void setCameraValueUpdateInterval(int msec);
    static int cameraValueUpdateInterval();
    static int defaultCameraValueUpdateInterval();

    static void setLockPropertiesEnable(bool isEnable);
    static bool isLockPropertiesEnable();
    static bool defaultIsLockPropertiesEnable();

    static void setLockPropertiesInterval(int msec);
    static int lockPropertiesInterval();
    static int defaultLockPropertiesInterval();

    static void setLockProperties(const QVariantMap &map);
    static QVariantMap lockProperties();

    static void setLockPropertiesPresets(const QVariantMap &map);
    static QVariantMap lockPropertiesPresets();

    static void setUserDefaultValues(const QVariantMap &map);
    static QVariantMap userDefaultValues();

    static void setUserPresets(const QVariantMap &map);
    static QVariantMap userPresets();

protected:
    explicit FSSettings();
    ~FSSettings() override;
};
