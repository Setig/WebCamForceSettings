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

#include "fscoresettings.h"

#include <QSettings>

#include <WebCamFS/Camera>

#define MAIN_SETTINGS_GROUP_NAME       "MainSettings"
#define STATISTICS_SETTINGS_GROUP_NAME "Statistics"

#define CAMERA_USER_NAMES_NAME            "UserNames"
#define CAMERA_BLACK_LIST_NAME            "BlackList"

#define USUALLY_AVAILABLE_CAMERAS_COUNT_NAME "UsuallyAvailableCamerasCount"
#define MAX_LOCK_PROPERTIES_NAME             "MaxLockProperties"

#define USUALLY_AVAILABLE_CAMERAS_COUNT_DEFAULT_VALUE 2
#define MAX_LOCK_PROPERTIES_DEFAULT_VALUE             fsCameraPropertiesCount()/4

static QSettings *statSettingsInstance = nullptr;

void FSCoreSettings::initialization()
{
    if (!statSettingsInstance) {
        statSettingsInstance = new QSettings(QSettings::IniFormat,
                                             QSettings::UserScope,
                                             "WebCamFS",
                                             "WebCamFS");
    }
}

void FSCoreSettings::uninitialization()
{
    if (statSettingsInstance) {
        delete statSettingsInstance;
        statSettingsInstance = nullptr;
    }
}

QSettings *FSCoreSettings::instance()
{
    initialization();
    return statSettingsInstance;
}

void FSCoreSettings::sync()
{
    if (statSettingsInstance)
        statSettingsInstance->sync();
}

void FSCoreSettings::setUsuallyAvailableCamerasCount(uint count)
{
    if (!fastInstance())
        return;

    beginStatisticsGroup();
    fastInstance()->setValue(USUALLY_AVAILABLE_CAMERAS_COUNT_NAME, count);
    endGroup();
}

uint FSCoreSettings::usuallyAvailableCamerasCount()
{
    if (!fastInstance())
        return USUALLY_AVAILABLE_CAMERAS_COUNT_DEFAULT_VALUE;

    beginStatisticsGroup();
    const uint value = fastInstance()->value(USUALLY_AVAILABLE_CAMERAS_COUNT_NAME,
                                             USUALLY_AVAILABLE_CAMERAS_COUNT_DEFAULT_VALUE).toUInt();
    endGroup();

    if (value == 0)
        return 1;

    return value;
}

uint FSCoreSettings::defaultUsuallyAvailableCamerasCount()
{
    return USUALLY_AVAILABLE_CAMERAS_COUNT_DEFAULT_VALUE;
}

void FSCoreSettings::setMaxLockProperties(uint count)
{
    if (!fastInstance())
        return;

    beginStatisticsGroup();
    fastInstance()->setValue(MAX_LOCK_PROPERTIES_NAME, count);
    endGroup();
}

uint FSCoreSettings::maxLockProperties()
{
    if (!fastInstance())
        return uint(MAX_LOCK_PROPERTIES_DEFAULT_VALUE);

    beginStatisticsGroup();
    const uint value = fastInstance()->value(MAX_LOCK_PROPERTIES_NAME,
                                             MAX_LOCK_PROPERTIES_DEFAULT_VALUE).toUInt();
    endGroup();

    if (value == 0)
        return 1;

    return value;
}

uint FSCoreSettings::defaultMaxLockProperties()
{
    return uint(MAX_LOCK_PROPERTIES_DEFAULT_VALUE);
}

void FSCoreSettings::setUserNames(const QVariantMap &map)
{
    if (!fastInstance())
        return;

    beginMainSettingsGroup();
    fastInstance()->setValue(CAMERA_USER_NAMES_NAME, map);
    endGroup();
}

QVariantMap FSCoreSettings::userNames()
{
    if (!fastInstance())
        return QVariantMap();

    beginMainSettingsGroup();
    const QVariantMap value = fastInstance()->value(CAMERA_USER_NAMES_NAME).toMap();
    endGroup();

    return value;
}

void FSCoreSettings::setBlackList(const QVariantList &list)
{
    if (!fastInstance())
        return;

    beginMainSettingsGroup();
    fastInstance()->setValue(CAMERA_BLACK_LIST_NAME, list);
    endGroup();
}

QVariantList FSCoreSettings::blackList()
{
    if (!fastInstance())
        return QVariantList();

    beginMainSettingsGroup();
    const QVariantList value = fastInstance()->value(CAMERA_BLACK_LIST_NAME).toList();
    endGroup();

    return value;
}

QString FSCoreSettings::replaceSettingName(const QString &orig)
{
    QString result;
    result.reserve(orig.size());

    for (QString::const_iterator it = orig.cbegin(); it != orig.cend(); ++it) {
        if (*it == ' ') {
            ++it;
            if (it != orig.cend()) {
                if (it->category() == QChar::Letter_Lowercase) {
                    result.push_back(it->toUpper());
                    continue;
                } else {
                    result.push_back(' ');
                }
            } else {
                break;
            }
        }

        result.push_back(*it);
    }

    return result;
}

FSCoreSettings::FSCoreSettings()
{
    // do nothing
}

FSCoreSettings::~FSCoreSettings()
{
    // do nothing
}

QSettings *FSCoreSettings::fastInstance()
{
    return statSettingsInstance;
}

void FSCoreSettings::beginMainSettingsGroup()
{
    statSettingsInstance->beginGroup(MAIN_SETTINGS_GROUP_NAME);
}

void FSCoreSettings::beginStatisticsGroup()
{
    statSettingsInstance->beginGroup(STATISTICS_SETTINGS_GROUP_NAME);
}

void FSCoreSettings::endGroup()
{
    statSettingsInstance->endGroup();
}
