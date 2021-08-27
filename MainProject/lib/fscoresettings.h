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

#include <QVariantMap>

class QString;
class QSettings;

class FS_LIB_EXPORT FSCoreSettings
{
public:
    static void initialization();
    static void uninitialization();

    static QSettings *instance();

    static void sync();

    static void setUsuallyAvailableCamerasCount(uint count);
    static uint usuallyAvailableCamerasCount();
    static uint defaultUsuallyAvailableCamerasCount();

    static void setMaxLockProperties(uint count);
    static uint maxLockProperties();
    static uint defaultMaxLockProperties();

    static void setUserNames(const QVariantMap &map);
    static QVariantMap userNames();

    static void setBlackList(const QVariantList &list);
    static QVariantList blackList();

    static QString replaceSettingName(const QString &orig);

protected:
    explicit FSCoreSettings();
    virtual ~FSCoreSettings();

    static QSettings *fastInstance();

    static void beginMainSettingsGroup();
    static void beginStatisticsGroup();
    static void endGroup();
};
