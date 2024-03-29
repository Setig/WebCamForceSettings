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

#include "fs_structs.h"

#include <QIODevice>
#include <QDataStream>

#include <windows.h>
#include <ks.h>
#include <ksmedia.h>

#include <WebCamFS/Camera>

#define FS_RANGE_PARAMS_MAGIC_NUMBER quint32(0x14FB8325)
#define FS_VALUE_PARAMS_MAGIC_NUMBER quint32(0xA0C05B34)

FSCameraData::FSCameraData() noexcept
    : m_pCap(nullptr)
{
    // do nothing
}

FSCameraData::FSCameraData(const DeviceName &name, const DevicePath &devicePath, IBaseFilter *pCap)
    : m_name(name)
    , m_devicePath(devicePath)
    , m_pCap(pCap)
{
    // do nothing
}

FSCameraData::FSCameraData(const FSCameraData &other) noexcept
    : m_name(other.name())
    , m_devicePath(other.devicePath())
    , m_pCap(other.pCap())
{
    // do nothing
}

DeviceName FSCameraData::name() const
{
    return m_name;
}

DevicePath FSCameraData::devicePath() const
{
    return m_devicePath;
}

IBaseFilter *FSCameraData::pCap() const
{
    return m_pCap;
}

void FSCameraData::release()
{
    FSCamera::releaseCameraData(*this);
}

bool FSCameraData::operator==(const FSCameraData &other) const noexcept
{
    return (this->m_name       == other.m_name       &&
            this->m_devicePath == other.m_devicePath &&
            this->m_pCap       == other.m_pCap);
}

bool FSCameraData::operator!=(const FSCameraData &other) const noexcept
{
    return !(*this == other);
}

FSCameraData &FSCameraData::operator=(const FSCameraData &other) noexcept
{
    this->m_name       = other.m_name;
    this->m_devicePath = other.m_devicePath;
    this->m_pCap       = other.m_pCap;

    return *this;
}

FSRangeParams::FSRangeParams() noexcept
    : m_isNull(true)
    , m_minValue(0)
    , m_maxValue(0)
    , m_stepValue(0)
    , m_value(0)
    , m_flags(KSPROPERTY_CAMERACONTROL_FLAGS_ABSOLUTE)
{
    // do nothing
}

FSRangeParams::FSRangeParams(long min,
                             long max,
                             long step,
                             long value,
                             long flags)
    : m_isNull(false)
    , m_minValue(min)
    , m_maxValue(max)
    , m_stepValue(step)
    , m_value(value)
    , m_flags(flags)
{
    // do nothing
}

FSRangeParams::FSRangeParams(const FSRangeParams &other) noexcept
    : m_isNull(other.isNull())
    , m_minValue(other.minValue())
    , m_maxValue(other.maxValue())
    , m_stepValue(other.stepValue())
    , m_value(other.value())
    , m_flags(other.flags())
{
    // do nothing
}

bool FSRangeParams::isNull() const
{
    return m_isNull;
}

long FSRangeParams::minValue() const
{
    return m_minValue;
}

long FSRangeParams::maxValue() const
{
    return m_maxValue;
}

long FSRangeParams::stepValue() const
{
    return m_stepValue;
}

long FSRangeParams::value() const
{
    return m_value;
}

long FSRangeParams::flags() const
{
    return m_flags;
}

bool FSRangeParams::isSupportAutoControl() const
{
    return (m_flags & KSPROPERTY_CAMERACONTROL_FLAGS_AUTO);
}

bool FSRangeParams::isSupportManualControl() const
{
    return (m_flags & KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL);
}

bool FSRangeParams::operator==(const FSRangeParams &other) const noexcept
{
    return (this->m_isNull == other.m_isNull &&
            this->m_minValue    == other.m_minValue    &&
            this->m_maxValue    == other.m_maxValue    &&
            this->m_stepValue   == other.m_stepValue   &&
            this->m_value  == other.m_value  &&
            this->m_flags  == other.m_flags);
}

bool FSRangeParams::operator!=(const FSRangeParams &other) const noexcept
{
    return !(*this == other);
}

FSRangeParams &FSRangeParams::operator=(const FSRangeParams &other) noexcept
{
    this->m_isNull = other.m_isNull;
    this->m_minValue    = other.m_minValue;
    this->m_maxValue    = other.m_maxValue;
    this->m_stepValue   = other.m_stepValue;
    this->m_value  = other.m_value;
    this->m_flags  = other.m_flags;

    return *this;
}

FSRangeParams FSRangeParams::fromByteArray(const QByteArray &byteArray, bool *isOk)
{
    QDataStream stream(byteArray);
    stream.setVersion(QDataStream::Qt_4_0);

    const quint32 magicNumber = FS_RANGE_PARAMS_MAGIC_NUMBER;
    quint32 storedMagicNumber;
    stream >> storedMagicNumber;
    if (storedMagicNumber != magicNumber) {
        if (isOk) { *isOk = false; }
        return FSRangeParams();
    }

    const quint16 currentMajorVersion = 1;
    quint16 majorVersion = 0;
    quint16 minorVersion = 0;

    stream >> majorVersion >> minorVersion;
    if (majorVersion > currentMajorVersion) {
        if (isOk) { *isOk = false; }
        return FSRangeParams();
    }

    bool isNull;
    qint32 min;
    qint32 max;
    qint32 step;
    qint32 value;
    qint32 flags;

    stream >> isNull >> min >> max >> step >> value >> flags;

    if (isNull) {
        if (isOk) { *isOk = true; }
        return FSRangeParams();
    }

    if (isOk) { *isOk = true; }
    return FSRangeParams(min, max, step, value, flags);
}

QByteArray FSRangeParams::toByteArray() const
{
    QByteArray byteArray;
    QDataStream stream(&byteArray, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_4_0);
    const quint32 magicNumber = FS_RANGE_PARAMS_MAGIC_NUMBER;
    const quint16 majorVersion = 1;
    const quint16 minorVersion = 0;
    const bool isNull  = this->isNull();
    const qint32 min   = this->minValue();
    const qint32 max   = this->maxValue();
    const qint32 step  = this->stepValue();
    const qint32 value = this->value();
    const qint32 flags = this->flags();

    stream << magicNumber
           << majorVersion
           << minorVersion
           << isNull
           << min
           << max
           << step
           << value
           << flags;

    return byteArray;
}

FSValueParams::FSValueParams() noexcept
    : m_isNull(true)
    , m_value(0)
    , m_flags(KSPROPERTY_CAMERACONTROL_FLAGS_ABSOLUTE)
{
    // do nothing
}

long FSValueParams::genAutoControlFlags()
{
    return KSPROPERTY_CAMERACONTROL_FLAGS_AUTO;
}

long FSValueParams::genManualControlFlags()
{
    return KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL;
}

FSValueParams::FSValueParams(long value, long flags)
    : m_isNull(false)
    , m_value(value)
    , m_flags(flags)
{
    // do nothing
}

FSValueParams::FSValueParams(long value)
    : m_isNull(false)
    , m_value(value)
    , m_flags(KSPROPERTY_CAMERACONTROL_FLAGS_ABSOLUTE)
{
    // do nothing
}

FSValueParams::FSValueParams(const FSValueParams &other) noexcept
    : m_isNull(other.isNull())
    , m_value(other.value())
    , m_flags(other.flags())
{
    // do nothing
}

bool FSValueParams::isNull() const
{
    return m_isNull;
}

long FSValueParams::value() const
{
    return m_value;
}

long FSValueParams::flags() const
{
    return m_flags;
}

bool FSValueParams::isAutoControl() const
{
    return (m_flags == KSPROPERTY_CAMERACONTROL_FLAGS_AUTO);
}

bool FSValueParams::isManualControl() const
{
    return (m_flags == KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL);
}

bool FSValueParams::operator==(const FSValueParams &other) const noexcept
{
    return (this->m_isNull == other.m_isNull &&
            this->m_value  == other.m_value  &&
            this->m_flags  == other.m_flags);
}

bool FSValueParams::operator!=(const FSValueParams &other) const noexcept
{
    return !(*this == other);
}

FSValueParams &FSValueParams::operator=(const FSValueParams &other) noexcept
{
    this->m_isNull = other.m_isNull;
    this->m_value  = other.m_value;
    this->m_flags  = other.m_flags;

    return *this;
}

FSValueParams FSValueParams::fromByteArray(const QByteArray &byteArray, bool *isOk)
{
    QDataStream stream(byteArray);
    stream.setVersion(QDataStream::Qt_4_0);

    const quint32 magicNumber = FS_VALUE_PARAMS_MAGIC_NUMBER;
    quint32 storedMagicNumber;
    stream >> storedMagicNumber;
    if (storedMagicNumber != magicNumber) {
        if (isOk) { *isOk = false; }
        return FSValueParams();
    }

    const quint16 currentMajorVersion = 1;
    quint16 majorVersion = 0;
    quint16 minorVersion = 0;

    stream >> majorVersion >> minorVersion;
    if (majorVersion > currentMajorVersion) {
        if (isOk) { *isOk = false; }
        return FSValueParams();
    }

    bool isNull;
    qint32 value;
    qint32 flags;

    stream >> isNull >> value >> flags;

    if (isNull) {
        if (isOk) { *isOk = true; }
        return FSValueParams();
    }

    if (isOk) { *isOk = true; }
    return FSValueParams(value, flags);
}

QByteArray FSValueParams::toByteArray() const
{
    QByteArray byteArray;
    QDataStream stream(&byteArray, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_4_0);
    const quint32 magicNumber = FS_VALUE_PARAMS_MAGIC_NUMBER;
    const quint16 majorVersion = 1;
    const quint16 minorVersion = 0;
    const bool isNull = this->isNull();
    const qint32 value = this->value();
    const qint32 flags = this->flags();

    stream << magicNumber
           << majorVersion
           << minorVersion
           << isNull
           << value
           << flags;

    return byteArray;
}

std::vector<FSCameraProperty> fsStatAllCameraProperties =
{ FSCameraProperty::Brightness,
  FSCameraProperty::Contrast,
  FSCameraProperty::Hue,
  FSCameraProperty::Saturation,
  FSCameraProperty::Sharpness,
  FSCameraProperty::Gamma,
  FSCameraProperty::ColorEnable,
  FSCameraProperty::WhiteBalance,
  FSCameraProperty::BacklightCompensation,
  FSCameraProperty::Gain,
  FSCameraProperty::PowerlineFrequency,
  FSCameraProperty::Pan,
  FSCameraProperty::Tilt,
  FSCameraProperty::Roll,
  FSCameraProperty::Zoom,
  FSCameraProperty::Exposure,
  FSCameraProperty::Iris,
  FSCameraProperty::Focus
};

QString fsGetEnumName(FSCameraProperty property)
{
    switch (property) {
    case FSCameraProperty::None:
        return QString();
    case FSCameraProperty::Brightness:
        return QStringLiteral("Brightness");
    case FSCameraProperty::Contrast:
        return QStringLiteral("Contrast");
    case FSCameraProperty::Hue:
        return QStringLiteral("Hue");
    case FSCameraProperty::Saturation:
        return QStringLiteral("Saturation");
    case FSCameraProperty::Sharpness:
        return QStringLiteral("Sharpness");
    case FSCameraProperty::Gamma:
        return QStringLiteral("Gamma");
    case FSCameraProperty::ColorEnable:
        return QStringLiteral("Color enable");
    case FSCameraProperty::WhiteBalance:
        return QStringLiteral("White balance");
    case FSCameraProperty::BacklightCompensation:
        return QStringLiteral("Backlight compensation");
    case FSCameraProperty::Gain:
        return QStringLiteral("Gain");
    case FSCameraProperty::PowerlineFrequency:
        return QStringLiteral("Powerline frequency");
    case FSCameraProperty::Pan:
        return QStringLiteral("Pan");
    case FSCameraProperty::Tilt:
        return QStringLiteral("Tilt");
    case FSCameraProperty::Roll:
        return QStringLiteral("Roll");
    case FSCameraProperty::Zoom:
        return QStringLiteral("Zoom");
    case FSCameraProperty::Exposure:
        return QStringLiteral("Exposure");
    case FSCameraProperty::Iris:
        return QStringLiteral("Iris");
    case FSCameraProperty::Focus:
        return QStringLiteral("Focus");
    }

    return QString();
}

FSCameraProperty fsReadNameEnum(const QString &propertyName)
{
    if (propertyName == QLatin1String("Brightness"))
        return FSCameraProperty::Brightness;
    else if (propertyName == QLatin1String("Contrast"))
        return FSCameraProperty::Contrast;
    else if (propertyName == QLatin1String("Hue"))
        return FSCameraProperty::Hue;
    else if (propertyName == QLatin1String("Saturation"))
        return FSCameraProperty::Saturation;
    else if (propertyName == QLatin1String("Sharpness"))
        return FSCameraProperty::Sharpness;
    else if (propertyName == QLatin1String("Gamma"))
        return FSCameraProperty::Gamma;
    else if (propertyName == QLatin1String("Color enable"))
        return FSCameraProperty::ColorEnable;
    else if (propertyName == QLatin1String("White balance"))
        return FSCameraProperty::WhiteBalance;
    else if (propertyName == QLatin1String("Backlight compensation"))
        return FSCameraProperty::BacklightCompensation;
    else if (propertyName == QLatin1String("Gain"))
        return FSCameraProperty::Gain;
    else if (propertyName == QLatin1String("Powerline frequency"))
        return FSCameraProperty::PowerlineFrequency;
    else if (propertyName == QLatin1String("Pan"))
        return FSCameraProperty::Pan;
    else if (propertyName == QLatin1String("Tilt"))
        return FSCameraProperty::Tilt;
    else if (propertyName == QLatin1String("Roll"))
        return FSCameraProperty::Roll;
    else if (propertyName == QLatin1String("Zoom"))
        return FSCameraProperty::Zoom;
    else if (propertyName == QLatin1String("Exposure"))
        return FSCameraProperty::Exposure;
    else if (propertyName == QLatin1String("Iris"))
        return FSCameraProperty::Iris;
    else if (propertyName == QLatin1String("Focus"))
        return FSCameraProperty::Focus;

    return FSCameraProperty::None;
}

std::vector<FSCameraProperty> fsAllCameraProperties()
{
    return fsStatAllCameraProperties;
}

size_t fsCameraPropertiesCount()
{
    return fsStatAllCameraProperties.size();
}
