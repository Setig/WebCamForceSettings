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

#include <QString>
#include <QMetaType>

typedef QString DeviceName;
typedef QString DevicePath;

class FSCamera;
struct IBaseFilter;

struct FS_LIB_EXPORT FSCameraData
{
    FSCameraData() noexcept;

    explicit FSCameraData(const DeviceName &name,
                          const DevicePath &devicePath,
                          IBaseFilter *pCap);

    FSCameraData(const FSCameraData &other) noexcept;

    DeviceName   name()       const;
    DevicePath   devicePath() const;
    IBaseFilter *pCap()       const;

    void release();

    bool operator==(const FSCameraData &other) const noexcept;
    bool operator!=(const FSCameraData &other) const noexcept;
    FSCameraData &operator=(const FSCameraData &other) noexcept;

private:
    DeviceName   m_name;
    DevicePath   m_devicePath;
    IBaseFilter *m_pCap;

    friend class FSCamera;
};
Q_DECLARE_METATYPE(FSCameraData);


struct FS_LIB_EXPORT FSRangeParams
{
    FSRangeParams() noexcept;

    explicit FSRangeParams(long min,
                           long max,
                           long step,
                           long value,
                           long flags);

    FSRangeParams(const FSRangeParams &other) noexcept;

    bool isNull()    const;
    long minValue()  const;
    long maxValue()  const;
    long stepValue() const;
    long value()     const;
    long flags()     const;

    bool isSupportAutoControl()   const;
    bool isSupportManualControl() const;

    bool operator==(const FSRangeParams &other) const noexcept;
    bool operator!=(const FSRangeParams &other) const noexcept;
    FSRangeParams &operator=(const FSRangeParams &other) noexcept;

    static FSRangeParams fromByteArray(const QByteArray &byteArray, bool *isOk = nullptr);
    QByteArray toByteArray() const;

private:
    bool m_isNull;
    long m_minValue;
    long m_maxValue;
    long m_stepValue;
    long m_value;
    long m_flags;
};


struct FS_LIB_EXPORT FSValueParams
{
    FSValueParams() noexcept;

    static long genAutoControlFlags();
    static long genManualControlFlags();

    explicit FSValueParams(long value, long flags);
    explicit FSValueParams(long value);

    FSValueParams(const FSValueParams &other) noexcept;

    bool isNull() const;
    long value()  const;
    long flags()  const;

    bool isAutoControl()   const;
    bool isManualControl() const;

    bool operator==(const FSValueParams &other) const noexcept;
    bool operator!=(const FSValueParams &other) const noexcept;
    FSValueParams &operator=(const FSValueParams &other) noexcept;

    static FSValueParams fromByteArray(const QByteArray &byteArray, bool *isOk = nullptr);
    QByteArray toByteArray() const;

private:
    bool m_isNull;
    long m_value;
    long m_flags;
};


enum FSCameraProperty
{
    None = 0,
    Brightness,
    Contrast,
    Hue,
    Saturation,
    Sharpness,
    Gamma,
    ColorEnable,
    WhiteBalance,
    BacklightCompensation,
    Gain,
    PowerlineFrequency,
    Pan,
    Tilt,
    Roll,
    Zoom,
    Exposure,
    Iris,
    Focus
};


QString                       FS_LIB_EXPORT fsGetEnumName(FSCameraProperty property);
FSCameraProperty              FS_LIB_EXPORT fsReadNameEnum(const QString &propertyName);
std::vector<FSCameraProperty> FS_LIB_EXPORT fsAllCameraProperties();
size_t                        FS_LIB_EXPORT fsCameraPropertiesCount();
