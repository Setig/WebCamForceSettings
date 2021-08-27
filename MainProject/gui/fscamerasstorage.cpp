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

#include "fscamerasstorage.h"

#include <WebCamFS/Settings>

class FSCamerasStoragePrivate
{
public:
    FSCamerasStoragePrivate();

    FSCameraPathValuesUMap      umapCameraUserDefaultValues;
    FSCameraPathUserPresetsUMap umapCameraUserPresets;
};

FSCamerasStoragePrivate::FSCamerasStoragePrivate()
{
    const size_t standartCamerasCount = FSSettings::usuallyAvailableCamerasCount();

    umapCameraUserDefaultValues.reserve(standartCamerasCount);
    umapCameraUserPresets.reserve(standartCamerasCount);
}

FSCamerasStorage::FSCamerasStorage(QObject *parent)
    : FSCoreCamerasStorage(parent)
{
    init();
}

FSCamerasStorage::~FSCamerasStorage()
{
    delete d;
    d = nullptr;
}

void FSCamerasStorage::saveAll()
{
    FSCoreCamerasStorage::saveAll();

    saveDefaultValueParams();
    saveCameraUserPresets();
}

void FSCamerasStorage::loadAll()
{
    FSCoreCamerasStorage::loadAll();

    loadDefaultValueParams();
    loadCameraUserPresets();
}

void FSCamerasStorage::setUserDefaultValues(const FSCameraPathValuesUMap &umapCameraPathValues)
{
    d->umapCameraUserDefaultValues = umapCameraPathValues;
}

void FSCamerasStorage::setUserDefaultValues(const DevicePath &devicePath,
                                            const FSCameraPropertyValuesUMap &umapCameraPropertyValues)
{
    if (umapCameraPropertyValues.empty())
        userDefaultValuesRemove(devicePath);
    else
        d->umapCameraUserDefaultValues.insert_or_assign(devicePath, umapCameraPropertyValues);
}

void FSCamerasStorage::setUserDefaultValue(const DevicePath &devicePath,
                                           FSCameraProperty property,
                                           const FSValueParams &valueParams)
{
    if (valueParams.isNull()) {
        userDefaultValueRemove(devicePath, property);
    } else {
        FSCameraPathValuesUMap::iterator devicePathIterator = d->umapCameraUserDefaultValues.find(devicePath);
        if (devicePathIterator != d->umapCameraUserDefaultValues.end()) {
            FSCameraPropertyValuesUMap &umapCameraPropertyValues = devicePathIterator->second;
            umapCameraPropertyValues.insert_or_assign(property, valueParams);
        } else {
            d->umapCameraUserDefaultValues.insert( { devicePath, { { property, valueParams } } } );
        }
    }
}

bool FSCamerasStorage::isUserDefaultValuesUsed(const DevicePath &devicePath)
{
    FSCameraPathValuesUMap::const_iterator iterator = d->umapCameraUserDefaultValues.find(devicePath);
    if (iterator != d->umapCameraUserDefaultValues.end()) {
        if (iterator->second.size() != 0) {
            return true;
        }
    }

    return false;
}

bool FSCamerasStorage::isUserDefaultValueUsed(const DevicePath &devicePath,
                                              FSCameraProperty property)
{
    FSCameraPathValuesUMap::const_iterator devicePathIterator = d->umapCameraUserDefaultValues.find(devicePath);
    if (devicePathIterator != d->umapCameraUserDefaultValues.end()) {
        const FSCameraPropertyValuesUMap &umapCameraPropertyValues = devicePathIterator->second;
        FSCameraPropertyValuesUMap::const_iterator propertyIterator = umapCameraPropertyValues.find(property);
        if (propertyIterator != umapCameraPropertyValues.end() && !propertyIterator->second.isNull()) {
            return true;
        }
    }

    return false;
}

FSCameraPathValuesUMap FSCamerasStorage::getUserDefaultValues() const
{
    return d->umapCameraUserDefaultValues;
}

FSCameraPropertyValuesUMap FSCamerasStorage::getUserDefaultValues(const DevicePath &devicePath) const
{
    FSCameraPathValuesUMap::const_iterator iterator = d->umapCameraUserDefaultValues.find(devicePath);
    if (iterator != d->umapCameraUserDefaultValues.end())
        return iterator->second;

    return FSCameraPropertyValuesUMap();
}

FSValueParams FSCamerasStorage::getUserDefaultValue(const DevicePath &devicePath,
                                                    FSCameraProperty property) const
{
    FSCameraPathValuesUMap::const_iterator devicePathIterator = d->umapCameraUserDefaultValues.find(devicePath);
    if (devicePathIterator != d->umapCameraUserDefaultValues.end()) {
        const FSCameraPropertyValuesUMap &umapCameraPropertyValues = devicePathIterator->second;
        FSCameraPropertyValuesUMap::const_iterator propertyIterator = umapCameraPropertyValues.find(property);
        if (propertyIterator != umapCameraPropertyValues.end()) {
            return propertyIterator->second;
        }
    }

    return FSValueParams();
}

void FSCamerasStorage::userDefaultValuesRemove(const DevicePath &devicePath)
{
    d->umapCameraUserDefaultValues.erase(devicePath);
}

void FSCamerasStorage::userDefaultValueRemove(const DevicePath &devicePath,
                                              FSCameraProperty property)
{
    FSCameraPathValuesUMap::iterator devicePathIterator = d->umapCameraUserDefaultValues.find(devicePath);
    if (devicePathIterator != d->umapCameraUserDefaultValues.end()) {
        FSCameraPropertyValuesUMap &umapCameraPropertyValues = devicePathIterator->second;
        FSCameraPropertyValuesUMap::iterator propertyIterator = umapCameraPropertyValues.find(property);
        if (propertyIterator != umapCameraPropertyValues.end()) {
            umapCameraPropertyValues.erase(propertyIterator);
        }

        if (umapCameraPropertyValues.empty()) {
            d->umapCameraUserDefaultValues.erase(devicePathIterator);
        }
    }
}

void FSCamerasStorage::userDefaultValuesClear()
{
    d->umapCameraUserDefaultValues.clear();
}

void FSCamerasStorage::saveDefaultValueParams()
{
    QVariantMap varMap;

    for (const auto&[devicePath, umapCameraPropertyValues] : d->umapCameraUserDefaultValues) {
        if (!devicePath.isEmpty() && !umapCameraPropertyValues.empty()) {
            QVariantMap propertyVarMap;

            for (const auto &[property, valueParams] : umapCameraPropertyValues) {
                if (!valueParams.isNull()) {
                    propertyVarMap.insert(fsGetEnumName(property),
                                          QVariant::fromValue(valueParams.toByteArray()));
                }
            }

            varMap.insert(devicePath, propertyVarMap);
        }
    }

    FSSettings::setUserDefaultValues(varMap);
}

void FSCamerasStorage::loadDefaultValueParams()
{
    const QVariantMap varMap = FSSettings::userDefaultValues();
    FSCameraPathValuesUMap umapCameraPathValues;

    for (QVariantMap::const_iterator deviceIterator = varMap.begin();
         deviceIterator != varMap.end();
         ++deviceIterator) {
        const DevicePath &devicePath = deviceIterator.key();

        if (!devicePath.isEmpty()) {
            const QVariantMap propertyVarMap = deviceIterator.value().toMap();

            if (!propertyVarMap.isEmpty()) {
                FSCameraPropertyValuesUMap umapCameraPropertyValues;

                for (QVariantMap::const_iterator propertyIterator = propertyVarMap.begin();
                     propertyIterator != propertyVarMap.end();
                     ++propertyIterator) {
                    const FSCameraProperty property = fsReadNameEnum(propertyIterator.key());

                    if (property != FSCameraProperty::None) {
                        bool isOk = false;
                        FSValueParams valueParams = FSValueParams::fromByteArray(propertyIterator.value().toByteArray(), &isOk);

                        if (isOk && !valueParams.isNull()) {
                            umapCameraPropertyValues.insert( { property, valueParams } );
                        }
                    }
                }

                umapCameraPathValues.insert( { devicePath, umapCameraPropertyValues } );
            }
        }
    }

    setUserDefaultValues(umapCameraPathValues);
}


void FSCamerasStorage::setCameraUserPresets(const FSCameraPathUserPresetsUMap &umapCameraUserPresets)
{
    for (const auto &[devicePath, umapCameraUserPresets] : d->umapCameraUserPresets) {
        if (umapCameraUserPresets.find(devicePath) == umapCameraUserPresets.end())
            cameraUserNameRemove(devicePath);
    }

    for (const auto &[devicePath, umapCameraUserPresets] : umapCameraUserPresets) {
        setCameraUserPresets(devicePath, umapCameraUserPresets);
    }
}

void FSCamerasStorage::setCameraUserPresets(const DevicePath &devicePath,
                                            const FSCameraUserPresetsUMap &umapCameraUserPresets)
{
    if (umapCameraUserPresets.empty()) {
        cameraUserPresetsRemove(devicePath);
    } else {
        FSCameraPathUserPresetsUMap::iterator iterator = d->umapCameraUserPresets.find(devicePath);
        if (iterator != d->umapCameraUserPresets.end()) {
            const FSCameraUserPresetsUMap oldUMapCameraUserPresets = iterator->second;

            bool isPresetNamesChanged = false;
            for (const auto &[presetName, propertyValues] : oldUMapCameraUserPresets) {
                if (umapCameraUserPresets.find(presetName) == umapCameraUserPresets.end()) {
                    isPresetNamesChanged = true;
                    break;
                }
            }

            if (!isPresetNamesChanged) {
                for (const auto &[presetName, propertyValues] : umapCameraUserPresets) {
                    if (oldUMapCameraUserPresets.find(presetName) == oldUMapCameraUserPresets.end()) {
                        isPresetNamesChanged = true;
                        break;
                    }
                }
            }

            iterator->second = umapCameraUserPresets;

            if (isPresetNamesChanged) {
                emit cameraUserPresetNamesChanged(devicePath);
            }
        } else {
            d->umapCameraUserPresets.insert( { devicePath, umapCameraUserPresets } );

            emit cameraUserPresetNamesChanged(devicePath);
        }
    }
}

void FSCamerasStorage::setCameraUserPreset(const DevicePath &devicePath,
                                           const QString &presetName,
                                           const FSCameraPropertyValuesUMap &umapCameraPropertyValues)
{
    if (umapCameraPropertyValues.empty()) {
        cameraUserPresetRemove(devicePath, presetName);
    } else {
        FSCameraPathUserPresetsUMap::iterator deviceIterator = d->umapCameraUserPresets.find(devicePath);
        if (deviceIterator != d->umapCameraUserPresets.end()) {
            FSCameraUserPresetsUMap &umapCameraUserPresets = deviceIterator->second;
            FSCameraUserPresetsUMap::iterator presetIterator = umapCameraUserPresets.find(presetName);
            if (presetIterator != umapCameraUserPresets.end()) {
                presetIterator->second = umapCameraPropertyValues;
            } else {
                umapCameraUserPresets.insert( { presetName, umapCameraPropertyValues } );

                emit cameraUserPresetNamesChanged(devicePath);
            }
        } else {
            d->umapCameraUserPresets.insert( { devicePath, { { presetName, umapCameraPropertyValues } } } );

            emit cameraUserPresetNamesChanged(devicePath);
        }
    }
}

void FSCamerasStorage::setCameraUserPreset(const DevicePath &devicePath,
                                           const QString &presetName,
                                           FSCameraProperty property,
                                           const FSValueParams &valueParams)
{
    if (valueParams.isNull()) {
        cameraUserPresetValueRemove(devicePath, presetName, property);
    } else {
        FSCameraPathUserPresetsUMap::iterator deviceIterator = d->umapCameraUserPresets.find(devicePath);
        if (deviceIterator != d->umapCameraUserPresets.end()) {
            FSCameraUserPresetsUMap &umapCameraUserPresets = deviceIterator->second;
            FSCameraUserPresetsUMap::iterator presetIterator = umapCameraUserPresets.find(presetName);
            if (presetIterator != umapCameraUserPresets.end()) {
                presetIterator->second.insert_or_assign(property, valueParams);
            } else {
                umapCameraUserPresets.insert( { presetName, { { property, valueParams } } } );

                emit cameraUserPresetNamesChanged(devicePath);
            }
        } else {
            d->umapCameraUserPresets.insert( { devicePath, { { presetName, { { property, valueParams } } } } } );

            emit cameraUserPresetNamesChanged(devicePath);
        }
    }
}

bool FSCamerasStorage::isCameraUserPresetsUsed(const DevicePath &devicePath)
{
    return (d->umapCameraUserPresets.find(devicePath) != d->umapCameraUserPresets.end());
}

bool FSCamerasStorage::isCameraUserPresetUsed(const DevicePath &devicePath,
                                              const QString &presetName)
{
    FSCameraPathUserPresetsUMap::iterator deviceIterator = d->umapCameraUserPresets.find(devicePath);
    if (deviceIterator != d->umapCameraUserPresets.end()) {
        FSCameraUserPresetsUMap &umapCameraUserPresets = deviceIterator->second;
        FSCameraUserPresetsUMap::iterator presetIterator = umapCameraUserPresets.find(presetName);
        return (presetIterator != umapCameraUserPresets.end());
    }

    return false;
}

bool FSCamerasStorage::isCameraUserPresetValueUsed(const DevicePath &devicePath,
                                                   const QString &presetName,
                                                   FSCameraProperty property)
{
    FSCameraPathUserPresetsUMap::iterator deviceIterator = d->umapCameraUserPresets.find(devicePath);
    if (deviceIterator != d->umapCameraUserPresets.end()) {
        FSCameraUserPresetsUMap &umapCameraUserPresets = deviceIterator->second;
        FSCameraUserPresetsUMap::iterator presetIterator = umapCameraUserPresets.find(presetName);
        if (presetIterator != umapCameraUserPresets.end()) {
            FSCameraPropertyValuesUMap &umapCameraPropertyValues = presetIterator->second;
            return (umapCameraPropertyValues.find(property) != umapCameraPropertyValues.end());
        }
    }

    return false;
}

FSCameraPathUserPresetsUMap FSCamerasStorage::getCameraUserPresets() const
{
    return d->umapCameraUserPresets;
}

std::vector<QString> FSCamerasStorage::getCameraUserPresetNames(const DevicePath &devicePath) const
{
    std::vector<QString> result;

    FSCameraPathUserPresetsUMap::iterator deviceIterator = d->umapCameraUserPresets.find(devicePath);
    if (deviceIterator != d->umapCameraUserPresets.end()) {
        FSCameraUserPresetsUMap &umapCameraUserPresets = deviceIterator->second;
        result.reserve(umapCameraUserPresets.size());
        for (const auto &[presetName, propertyValues] : umapCameraUserPresets) {
            result.push_back(presetName);
        }
    }

    return result;
}

FSCameraUserPresetsUMap FSCamerasStorage::getCameraUserPresets(const DevicePath &devicePath) const
{
    FSCameraPathUserPresetsUMap::iterator deviceIterator = d->umapCameraUserPresets.find(devicePath);
    if (deviceIterator != d->umapCameraUserPresets.end()) {
        return deviceIterator->second;
    }

    return FSCameraUserPresetsUMap();
}

FSCameraPropertyValuesUMap FSCamerasStorage::getCameraUserPreset(const DevicePath &devicePath,
                                                                 const QString &presetName) const
{
    FSCameraPathUserPresetsUMap::iterator deviceIterator = d->umapCameraUserPresets.find(devicePath);
    if (deviceIterator != d->umapCameraUserPresets.end()) {
        FSCameraUserPresetsUMap &umapCameraUserPresets = deviceIterator->second;
        FSCameraUserPresetsUMap::iterator presetIterator = umapCameraUserPresets.find(presetName);
        if (presetIterator != umapCameraUserPresets.end()) {
            return presetIterator->second;
        }
    }

    return FSCameraPropertyValuesUMap();
}

FSValueParams FSCamerasStorage::getCameraUserPresetValue(const DevicePath &devicePath,
                                                         const QString &presetName,
                                                         FSCameraProperty property) const
{
    FSCameraPathUserPresetsUMap::iterator deviceIterator = d->umapCameraUserPresets.find(devicePath);
    if (deviceIterator != d->umapCameraUserPresets.end()) {
        FSCameraUserPresetsUMap &umapCameraUserPresets = deviceIterator->second;
        FSCameraUserPresetsUMap::iterator presetIterator = umapCameraUserPresets.find(presetName);
        if (presetIterator != umapCameraUserPresets.end()) {
            FSCameraPropertyValuesUMap &umapCameraPropertyValues = presetIterator->second;
            FSCameraPropertyValuesUMap::iterator propertyIterator = umapCameraPropertyValues.find(property);
            if (propertyIterator != umapCameraPropertyValues.end()) {
                return propertyIterator->second;
            }
        }
    }

    return FSValueParams();
}

void FSCamerasStorage::cameraUserPresetsRemove(const DevicePath &devicePath)
{
    d->umapCameraUserPresets.erase(devicePath);

    emit cameraUserPresetNamesChanged(devicePath);
}

void FSCamerasStorage::cameraUserPresetRemove(const DevicePath &devicePath,
                                              const QString &presetName)
{
    FSCameraPathUserPresetsUMap::iterator deviceIterator = d->umapCameraUserPresets.find(devicePath);
    if (deviceIterator != d->umapCameraUserPresets.end()) {
        FSCameraUserPresetsUMap &umapCameraUserPresets = deviceIterator->second;
        FSCameraUserPresetsUMap::iterator presetIterator = umapCameraUserPresets.find(presetName);
        if (presetIterator != umapCameraUserPresets.end()) {
            umapCameraUserPresets.erase(presetIterator);

            emit cameraUserPresetNamesChanged(devicePath);
        }

        if (umapCameraUserPresets.empty()) {
            d->umapCameraUserPresets.erase(deviceIterator);
        }
    }
}

void FSCamerasStorage::cameraUserPresetValueRemove(const DevicePath &devicePath,
                                                   const QString &presetName,
                                                   FSCameraProperty property)
{
    FSCameraPathUserPresetsUMap::iterator deviceIterator = d->umapCameraUserPresets.find(devicePath);
    if (deviceIterator != d->umapCameraUserPresets.end()) {
        FSCameraUserPresetsUMap &umapCameraUserPresets = deviceIterator->second;
        FSCameraUserPresetsUMap::iterator presetIterator = umapCameraUserPresets.find(presetName);
        if (presetIterator != umapCameraUserPresets.end()) {
            FSCameraPropertyValuesUMap &umapCameraPropertyValues = presetIterator->second;
            FSCameraPropertyValuesUMap::iterator propertyIterator = umapCameraPropertyValues.find(property);
            if (propertyIterator != umapCameraPropertyValues.end()) {
                umapCameraPropertyValues.erase(propertyIterator);
            }

            if (umapCameraPropertyValues.empty()) {
                umapCameraUserPresets.erase(presetName);

                emit cameraUserPresetNamesChanged(devicePath);
            }
        }

        if (umapCameraUserPresets.empty()) {
            d->umapCameraUserPresets.erase(deviceIterator);
        }
    }
}

void FSCamerasStorage::cameraUserPresetsClear()
{
    if (d->umapCameraUserPresets.empty())
        return;

    std::vector<DevicePath> devicePaths;
    devicePaths.reserve(d->umapCameraUserPresets.size());
    for (const auto &[devicePath, umapCameraUserPreset] : d->umapCameraUserPresets) {
        devicePaths.push_back(devicePath);
    }

    d->umapCameraUserPresets.clear();

    for (const DevicePath &devicePath : devicePaths) {
        emit cameraUserPresetNamesChanged(devicePath);
    }
}

void FSCamerasStorage::saveCameraUserPresets()
{
    QVariantMap varMap;

    for (const auto &[devicePath, umapCameraUserPreset] : d->umapCameraUserPresets) {
        if (!devicePath.isEmpty() && !umapCameraUserPreset.empty()) {
            QVariantMap presetVarMap;

            for (const auto &[presetName, umapPropertyValueParams] : umapCameraUserPreset) {
                if (!presetName.isEmpty() && !umapPropertyValueParams.empty()) {
                    QVariantMap propertyVarMap;

                    for (const auto &[property, valueParams] : umapPropertyValueParams) {
                        if (!valueParams.isNull()) {
                            propertyVarMap.insert(fsGetEnumName(property),
                                                  QVariant::fromValue(valueParams.toByteArray()));
                        }
                    }

                    presetVarMap.insert(presetName, propertyVarMap);
                }
            }

            varMap.insert(devicePath, presetVarMap);
        }
    }

    FSSettings::setUserPresets(varMap);
}

void FSCamerasStorage::loadCameraUserPresets()
{
    const QVariantMap varMap = FSSettings::userPresets();
    FSCameraPathUserPresetsUMap umapCameraPathUserPresets;

    for (QVariantMap::const_iterator deviceIterator = varMap.begin();
         deviceIterator != varMap.end();
         ++deviceIterator) {
        const DevicePath &devicePath = deviceIterator.key();

        if (!devicePath.isEmpty()) {
            const QVariantMap presetVarMap = deviceIterator.value().toMap();

            if (!presetVarMap.isEmpty()) {
                FSCameraUserPresetsUMap umapCameraUserPresets;
                for (QVariantMap::const_iterator presetIterator = presetVarMap.begin();
                     presetIterator != presetVarMap.end();
                     ++presetIterator) {
                    const QString &presetName = presetIterator.key();

                    if (!presetName.isEmpty()) {
                        const QVariantMap propertyVarMap = presetIterator.value().toMap();

                        if (!propertyVarMap.isEmpty()) {
                            FSCameraPropertyValuesUMap umapCameraPropertyValues;

                            for (QVariantMap::const_iterator propertyIterator = propertyVarMap.begin();
                                 propertyIterator != propertyVarMap.end();
                                 ++propertyIterator) {
                                const FSCameraProperty property = fsReadNameEnum(propertyIterator.key());

                                if (property != FSCameraProperty::None) {
                                    bool isOk = false;
                                    FSValueParams valueParams = FSValueParams::fromByteArray(propertyIterator.value().toByteArray(), &isOk);

                                    if (isOk && !valueParams.isNull()) {
                                        umapCameraPropertyValues.insert( { property, valueParams } );
                                    }
                                }
                            }

                            umapCameraUserPresets.insert( { presetName, umapCameraPropertyValues } );
                        }
                    }
                }

                umapCameraPathUserPresets.insert( { devicePath, umapCameraUserPresets } );
            }
        }
    }

    setCameraUserPresets(umapCameraPathUserPresets);
}

void FSCamerasStorage::init()
{
    d = new FSCamerasStoragePrivate();
}
