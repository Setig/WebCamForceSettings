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

#include "fssettings.h"

#include <QLocale>
#include <QSettings>

#ifdef BUILD_WITH_Q_EASY_SETTINGS
#include "qeasysettings.hpp"
#endif // BUILD_WITH_Q_EASY_SETTINGS

#define CURRENT_LOCALE_NAME               QStringLiteral("CurrentLocale")
#define STYLE_NAME                        QStringLiteral("Style")
#define CAMERA_DETECTION_ENABLE_NAME      QStringLiteral("CameraDetectionEnable")
#define CAMERA_DETECTION_INTERVAL_NAME    QStringLiteral("CameraDetectionInterval")
#define CAMERA_VALUE_UPDATE_ENABLE_NAME   QStringLiteral("CameraValueUpdateEnable")
#define CAMERA_VALUE_UPDATE_INTERVAL_NAME QStringLiteral("CameraValueUpdateInterval")
#define LOCK_PROPERTIES_ENABLE_NAME       QStringLiteral("LockPropertiesEnable")
#define LOCK_PROPERTIES_INTERVAL_NAME     QStringLiteral("LockPropertiesInterval")
#define LOCK_PROPERTIES_NAME              QStringLiteral("LockProperties")
#define LOCK_PROPERTIES_PRESETS_NAME      QStringLiteral("LockPropertiesPresets")
#define CAMERA_USER_DEFAULT_VALUES_NAME   QStringLiteral("UserDefaultValues")
#define CAMERA_USER_PRESETS_NAME          QStringLiteral("UserPresets")

#define CAMERA_DETECTION_ENABLE_DEFAULT_VALUE      true
#define CAMERA_DETECTION_INTERVAL_DEFAULT_VALUE    1000
#define CAMERA_VALUE_UPDATE_ENABLE_DEFAULT_VALUE   true
#define CAMERA_VALUE_UPDATE_INTERVAL_DEFAULT_VALUE 1000
#define LOCK_PROPERTIES_ENABLE_DEFAULT_VALUE       true
#define LOCK_PROPERTIES_INTERVAL_DEFAULT_VALUE     250

void FSSettings::initialization()
{
    FSCoreSettings::initialization();

#ifdef BUILD_WITH_Q_EASY_SETTINGS
    QEasySettings::init(QEasySettings::Format::iniFormat,
                        fastInstance()->fileName());
#endif // BUILD_WITH_Q_EASY_SETTINGS
}

void FSSettings::uninitialization()
{
    FSCoreSettings::uninitialization();
}

void FSSettings::setCurrentLocale(const QLocale &locale)
{
    if (!fastInstance())
        return;

    beginMainSettingsGroup();
    fastInstance()->setValue(CURRENT_LOCALE_NAME, locale);
    endGroup();
}

QLocale FSSettings::currentLocale()
{
    if (!fastInstance())
        return defaultCurrentLocale();

    beginMainSettingsGroup();
    const QLocale value = fastInstance()->value(CURRENT_LOCALE_NAME,
                                                defaultCurrentLocale()).value<QLocale>();
    endGroup();
    return value;
}

QLocale FSSettings::defaultCurrentLocale()
{
    QLocale locale = QLocale::system();

    if (locale.language() == QLocale::AnyLanguage) {
        locale = QLocale(QLocale::English);
    }

    return locale;
}

void FSSettings::setCustomStyleIndex(int styleIndex)
{
#ifdef BUILD_WITH_Q_EASY_SETTINGS
    if (styleIndex == -1)
        styleIndex = defaultCustomStyleIndex();

    QEasySettings::writeSettings(getMainSettingsGroupName(), STYLE_NAME, styleIndex);
#else
    Q_UNUSED(styleIndex);
#endif // BUILD_WITH_Q_EASY_SETTINGS
}

int FSSettings::customStyleIndex()
{
#ifdef BUILD_WITH_Q_EASY_SETTINGS
    QVariant value = QEasySettings::readSettings(getMainSettingsGroupName(), STYLE_NAME);

    if (value.isNull())
        return defaultCustomStyleIndex();

    return value.toInt();
#else
    return -1;
#endif // BUILD_WITH_Q_EASY_SETTINGS
}

int FSSettings::defaultCustomStyleIndex()
{
#ifdef BUILD_WITH_Q_EASY_SETTINGS
    return int(QEasySettings::Style::autoFusion);
#else
    return -1;
#endif // BUILD_WITH_Q_EASY_SETTINGS
}

void FSSettings::setCameraDetectionEnable(bool isEnable)
{
    if (!fastInstance())
        return;

    beginMainSettingsGroup();
    fastInstance()->setValue(CAMERA_DETECTION_ENABLE_NAME, isEnable);
    endGroup();
}

bool FSSettings::isCameraDetectionEnable()
{
    if (!fastInstance())
        return CAMERA_DETECTION_ENABLE_DEFAULT_VALUE;

    beginMainSettingsGroup();
    const bool value = fastInstance()->value(CAMERA_DETECTION_ENABLE_NAME,
                                             CAMERA_DETECTION_ENABLE_DEFAULT_VALUE).toBool();
    endGroup();
    return value;
}

bool FSSettings::defaultIsCameraDetectionEnable()
{
    return CAMERA_DETECTION_ENABLE_DEFAULT_VALUE;
}

void FSSettings::setCameraDetectionInterval(int msec)
{
    if (!fastInstance())
        return;

    beginMainSettingsGroup();
    fastInstance()->setValue(CAMERA_DETECTION_INTERVAL_NAME, msec);
    endGroup();
}

int FSSettings::cameraDetectionInterval()
{
    if (!fastInstance())
        return CAMERA_DETECTION_INTERVAL_DEFAULT_VALUE;

    beginMainSettingsGroup();
    const int value = fastInstance()->value(CAMERA_DETECTION_INTERVAL_NAME,
                                            CAMERA_DETECTION_INTERVAL_DEFAULT_VALUE).toInt();
    endGroup();
    return value;
}

int FSSettings::defaultCameraDetectionInterval()
{
    return CAMERA_DETECTION_INTERVAL_DEFAULT_VALUE;
}

void FSSettings::setCameraValueUpdateEnable(bool isEnable)
{
    if (!fastInstance())
        return;

    beginMainSettingsGroup();
    fastInstance()->setValue(CAMERA_VALUE_UPDATE_ENABLE_NAME, isEnable);
    endGroup();
}

bool FSSettings::isCameraValueUpdateEnable()
{
    if (!fastInstance())
        return CAMERA_VALUE_UPDATE_ENABLE_DEFAULT_VALUE;

    beginMainSettingsGroup();
    const bool value = fastInstance()->value(CAMERA_VALUE_UPDATE_ENABLE_NAME,
                                             CAMERA_VALUE_UPDATE_ENABLE_DEFAULT_VALUE).toBool();
    endGroup();
    return value;
}

bool FSSettings::defaultIsCameraValueUpdateEnable()
{
    return CAMERA_VALUE_UPDATE_ENABLE_DEFAULT_VALUE;
}

void FSSettings::setCameraValueUpdateInterval(int msec)
{
    if (!fastInstance())
        return;

    beginMainSettingsGroup();
    fastInstance()->setValue(CAMERA_VALUE_UPDATE_INTERVAL_NAME, msec);
    endGroup();
}

int FSSettings::cameraValueUpdateInterval()
{
    if (!fastInstance())
        return CAMERA_VALUE_UPDATE_INTERVAL_DEFAULT_VALUE;

    beginMainSettingsGroup();
    const int value = fastInstance()->value(CAMERA_VALUE_UPDATE_INTERVAL_NAME,
                                            CAMERA_VALUE_UPDATE_INTERVAL_DEFAULT_VALUE).toInt();
    endGroup();
    return value;
}

int FSSettings::defaultCameraValueUpdateInterval()
{
    return CAMERA_VALUE_UPDATE_INTERVAL_DEFAULT_VALUE;
}

void FSSettings::setLockPropertiesEnable(bool isEnable)
{
    if (!fastInstance())
        return;

    beginMainSettingsGroup();
    fastInstance()->setValue(LOCK_PROPERTIES_ENABLE_NAME, isEnable);
    endGroup();
}

bool FSSettings::isLockPropertiesEnable()
{
    if (!fastInstance())
        return LOCK_PROPERTIES_ENABLE_DEFAULT_VALUE;

    beginMainSettingsGroup();
    const bool value = fastInstance()->value(LOCK_PROPERTIES_ENABLE_NAME,
                                             LOCK_PROPERTIES_ENABLE_DEFAULT_VALUE).toBool();
    endGroup();
    return value;
}

bool FSSettings::defaultIsLockPropertiesEnable()
{
    return LOCK_PROPERTIES_ENABLE_DEFAULT_VALUE;
}

void FSSettings::setLockPropertiesInterval(int msec)
{
    if (!fastInstance())
        return;

    beginMainSettingsGroup();
    fastInstance()->setValue(LOCK_PROPERTIES_INTERVAL_NAME, msec);
    endGroup();
}

int FSSettings::lockPropertiesInterval()
{
    if (!fastInstance())
        return LOCK_PROPERTIES_INTERVAL_DEFAULT_VALUE;

    beginMainSettingsGroup();
    const int value = fastInstance()->value(LOCK_PROPERTIES_INTERVAL_NAME,
                                            LOCK_PROPERTIES_INTERVAL_DEFAULT_VALUE).toInt();
    endGroup();
    return value;
}

int FSSettings::defaultLockPropertiesInterval()
{
    return LOCK_PROPERTIES_INTERVAL_DEFAULT_VALUE;
}

void FSSettings::setLockProperties(const QVariantMap &map)
{
    if (!fastInstance())
        return;

    beginMainSettingsGroup();
    fastInstance()->setValue(LOCK_PROPERTIES_NAME, map);
    endGroup();
}

QVariantMap FSSettings::lockProperties()
{
    if (!fastInstance())
        return QVariantMap();

    beginMainSettingsGroup();
    const QVariantMap value = fastInstance()->value(LOCK_PROPERTIES_NAME).toMap();
    endGroup();

    return value;
}

void FSSettings::setLockPropertiesPresets(const QVariantMap &map)
{
    if (!fastInstance())
        return;

    beginMainSettingsGroup();
    fastInstance()->setValue(LOCK_PROPERTIES_PRESETS_NAME, map);
    endGroup();
}

QVariantMap FSSettings::lockPropertiesPresets()
{
    if (!fastInstance())
        return QVariantMap();

    beginMainSettingsGroup();
    const QVariantMap value = fastInstance()->value(LOCK_PROPERTIES_PRESETS_NAME).toMap();
    endGroup();

    return value;
}

void FSSettings::setUserDefaultValues(const QVariantMap &map)
{
    if (!fastInstance())
        return;

    beginMainSettingsGroup();
    fastInstance()->setValue(CAMERA_USER_DEFAULT_VALUES_NAME, map);
    endGroup();
}

QVariantMap FSSettings::userDefaultValues()
{
    if (!fastInstance())
        return QVariantMap();

    beginMainSettingsGroup();
    const QVariantMap value = fastInstance()->value(CAMERA_USER_DEFAULT_VALUES_NAME).toMap();
    endGroup();

    return value;
}

void FSSettings::setUserPresets(const QVariantMap &map)
{
    if (!fastInstance())
        return;

    beginMainSettingsGroup();
    fastInstance()->setValue(CAMERA_USER_PRESETS_NAME, map);
    endGroup();
}

QVariantMap FSSettings::userPresets()
{
    if (!fastInstance())
        return QVariantMap();

    beginMainSettingsGroup();
    const QVariantMap value = fastInstance()->value(CAMERA_USER_PRESETS_NAME).toMap();
    endGroup();

    return value;
}

FSSettings::FSSettings() : FSCoreSettings()
{
    // do nothing
}

FSSettings::~FSSettings()
{
    // do nothing
}
