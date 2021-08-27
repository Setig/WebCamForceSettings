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

#include "fslockpropertiesmanager.h"

#include <QTimer>
#include <QSettings>

#include <WebCamFS/Settings>
#include <WebCamFS/CamerasStorage>

typedef std::unordered_map<DevicePath, QString> FSCameraPathPresetsUmap;

class FSLockPropertiesManagerPrivate
{
public:
    FSLockPropertiesManagerPrivate();

    FSCamerasStorage *camerasStorage;

    bool isLockPropertiesEnabled;

    QTimer *sendLockedValuesTimer;
    QTimer *lockProperiesPauseTimer;

    FSCameraPathValuesUMap  umapManualLockCamerasParams;
    FSCameraPathPresetsUmap umapPresetsLockCameras;

    void manualLockProperty(FSLockPropertiesManager *manager,
                            const DevicePath &devicePath,
                            FSCameraProperty property,
                            const FSValueParams &valueParams);

    void manualUnlockProperty(FSLockPropertiesManager *manager,
                              const DevicePath &devicePath,
                              FSCameraProperty property);
};

FSLockPropertiesManagerPrivate::FSLockPropertiesManagerPrivate()
    : camerasStorage(nullptr)
    , isLockPropertiesEnabled(true)
    , sendLockedValuesTimer(nullptr)
    , lockProperiesPauseTimer(nullptr)
{
    const size_t standartCamerasCount = FSSettings::usuallyAvailableCamerasCount();

    umapManualLockCamerasParams.reserve(standartCamerasCount);
    umapPresetsLockCameras.reserve(standartCamerasCount);
}

void FSLockPropertiesManagerPrivate::manualLockProperty(FSLockPropertiesManager *manager,
                                                        const DevicePath &devicePath,
                                                        FSCameraProperty property,
                                                        const FSValueParams &valueParams)
{
    if (!camerasStorage)
        return;

    FSCamera *camera = camerasStorage->findCameraByDevicePath(devicePath);

    if (!camera)
        return;

    FSCameraPathValuesUMap::iterator lockPropertiesIterator = umapManualLockCamerasParams.find(devicePath);

    if (lockPropertiesIterator == umapManualLockCamerasParams.end()) {
        FSCameraPropertyValuesUMap lockPropertiesUMapParams;
        lockPropertiesUMapParams.reserve(FSSettings::maxLockProperties());

        umapManualLockCamerasParams.insert( { devicePath, lockPropertiesUMapParams } );

        if (camera) {
            emit manager->lockedCamera(camera);
        }

        lockPropertiesIterator = umapManualLockCamerasParams.find(devicePath);
    }

    FSCameraPropertyValuesUMap &umapCameraPropertyValues = lockPropertiesIterator->second;
    umapCameraPropertyValues.insert_or_assign(property, valueParams);

    if (camera) {
        emit manager->lockedProperty(camera, property);
    }

    manager->updateTimerStatus();
}

void FSLockPropertiesManagerPrivate::manualUnlockProperty(FSLockPropertiesManager *manager,
                                                          const DevicePath &devicePath,
                                                          FSCameraProperty property)
{
    if (!camerasStorage)
        return;

    FSCamera *camera = camerasStorage->findCameraByDevicePath(devicePath);

    if (!camera)
        return;

    FSCameraPathValuesUMap::iterator iterator = umapManualLockCamerasParams.find(devicePath);
    if (iterator == umapManualLockCamerasParams.end())
        return;

    FSCameraPropertyValuesUMap &umapCameraPropertyValues = iterator->second;
    umapCameraPropertyValues.erase(property);

    if (umapCameraPropertyValues.empty()) {
        umapManualLockCamerasParams.erase(iterator);
        emit manager->unlockedCamera(camera);
    }

    emit manager->unlockedProperty(camera, property);

    manager->updateTimerStatus();
}

FSLockPropertiesManager::FSLockPropertiesManager(QObject *parent) : QObject(parent)
{
    init();
}

FSLockPropertiesManager::~FSLockPropertiesManager()
{
    delete d->sendLockedValuesTimer;

    if (d->lockProperiesPauseTimer)
        delete d->lockProperiesPauseTimer;

    // Update statistics param
    {
        uint count = 1;

        for (const auto &[camera, lockedPropertiesValues] : d->umapManualLockCamerasParams) {
            uint size = lockedPropertiesValues.size();

            if (count < size)
                count = size;
        }

        FSSettings::setMaxLockProperties(count);
    }

    delete d;
    d = nullptr;
}

void FSLockPropertiesManager::setCamerasStorage(FSCamerasStorage *camerasStorage)
{
    if (d->camerasStorage != camerasStorage) {
        if (d->camerasStorage) {
            disconnect(d->camerasStorage, &FSCamerasStorage::addedCamera,
                       this,              &FSLockPropertiesManager::addedCamera);
            disconnect(d->camerasStorage, &FSCamerasStorage::removedCamera,
                       this,              &FSLockPropertiesManager::removedCamera);

            disconnect(d->camerasStorage, &FSCamerasStorage::cameraUserPresetNamesChanged,
                       this,              &FSLockPropertiesManager::updateCurrentPreset);
        }

        d->camerasStorage = camerasStorage;

        if (d->camerasStorage) {
            connect(d->camerasStorage, &FSCamerasStorage::addedCamera,
                    this,              &FSLockPropertiesManager::addedCamera);
            connect(d->camerasStorage, &FSCamerasStorage::removedCamera,
                    this,              &FSLockPropertiesManager::removedCamera);

            connect(d->camerasStorage, &FSCamerasStorage::cameraUserPresetNamesChanged,
                    this,              &FSLockPropertiesManager::updateCurrentPreset);
        }
    }
}

FSCamerasStorage *FSLockPropertiesManager::camerasStorage() const
{
    return d->camerasStorage;
}

bool FSLockPropertiesManager::isExistLockedProperties() const
{
    return (!d->umapManualLockCamerasParams.empty() || !d->umapPresetsLockCameras.empty());
}

bool FSLockPropertiesManager::isLockPropertiesEnable() const
{
    return d->isLockPropertiesEnabled;
}

void FSLockPropertiesManager::setLockPropertiesEnable(bool isEnable)
{
    if (d->isLockPropertiesEnabled != isEnable) {
        lockProperiesPauseAbort();

        d->isLockPropertiesEnabled = isEnable;

        updateTimerStatus();

        emit lockPropertiesEnableChanged(isEnable);
    }
}

bool FSLockPropertiesManager::isLockedLeastOneProperty(FSCamera *camera) const
{
    if (!camera)
        return false;

    return (d->umapPresetsLockCameras.find(camera->devicePath()) != d->umapPresetsLockCameras.end() ||
            d->umapManualLockCamerasParams.find(camera->devicePath()) != d->umapManualLockCamerasParams.end());
}

bool FSLockPropertiesManager::isLockedProperty(FSCamera *camera, FSCameraProperty property) const
{
    if (!camera)
        return false;

    const QString &devicePath = camera->devicePath();

    const FSCameraPathPresetsUmap::iterator &cameraPresetIterator = d->umapPresetsLockCameras.find(devicePath);
    if (cameraPresetIterator != d->umapPresetsLockCameras.end()) {
        const FSCameraPropertyValuesUMap umapCameraPropertyValues = d->camerasStorage->getCameraUserPreset(devicePath, cameraPresetIterator->second);
        return (umapCameraPropertyValues.find(property) != umapCameraPropertyValues.end());
    }

    const FSCameraPathValuesUMap::iterator &cameraManualIterator = d->umapManualLockCamerasParams.find(devicePath);
    if (cameraManualIterator != d->umapManualLockCamerasParams.end()) {
        const FSCameraPropertyValuesUMap &umapCameraPropertyValues = cameraManualIterator->second;
        return (umapCameraPropertyValues.find(property) != umapCameraPropertyValues.end());
    }

    return false;
}

FSValueParams FSLockPropertiesManager::lockedValueParams(FSCamera *camera, FSCameraProperty property) const
{
    if (!camera)
        return FSValueParams();

    const QString &devicePath = camera->devicePath();

    const FSCameraPathPresetsUmap::iterator &cameraPresetIterator = d->umapPresetsLockCameras.find(devicePath);
    if (cameraPresetIterator != d->umapPresetsLockCameras.end()) {
        const FSCameraPropertyValuesUMap umapCameraPropertyValues = d->camerasStorage->getCameraUserPreset(devicePath, cameraPresetIterator->second);
        const FSCameraPropertyValuesUMap::const_iterator &propertyIterator = umapCameraPropertyValues.find(property);
        if (propertyIterator != umapCameraPropertyValues.end()) {
            return propertyIterator->second;
        }
    }

    const FSCameraPathValuesUMap::iterator &cameraManualIterator = d->umapManualLockCamerasParams.find(devicePath);
    if (cameraManualIterator != d->umapManualLockCamerasParams.end()) {
        const FSCameraPropertyValuesUMap &umapCameraPropertyValues = cameraManualIterator->second;
        const FSCameraPropertyValuesUMap::const_iterator &propertyIterator = umapCameraPropertyValues.find(property);
        if (propertyIterator != umapCameraPropertyValues.end()) {
            return propertyIterator->second;
        }
    }

    return FSValueParams();
}

bool FSLockPropertiesManager::isContaintsManualLockProperties(FSCamera *camera) const
{
    if (!camera)
        return false;

    return (d->umapManualLockCamerasParams.find(camera->devicePath()) != d->umapManualLockCamerasParams.end());
}

bool FSLockPropertiesManager::isContaintsPresetLock(FSCamera *camera) const
{
    if (!camera)
        return false;

    return (d->umapPresetsLockCameras.find(camera->devicePath()) != d->umapPresetsLockCameras.end());
}

FSLockPropertiesManager::LockMode FSLockPropertiesManager::currentLockMode(FSCamera *camera) const
{
    if (!camera)
        return FSLockPropertiesManager::NoneLockMode;

    if (d->umapPresetsLockCameras.find(camera->devicePath()) != d->umapPresetsLockCameras.end()) {
        return FSLockPropertiesManager::PresetLockMode;
    }

    if (d->umapManualLockCamerasParams.find(camera->devicePath()) != d->umapManualLockCamerasParams.end()) {
        return FSLockPropertiesManager::ManualLockMode;
    }

    return FSLockPropertiesManager::NoneLockMode;
}

QString FSLockPropertiesManager::lockPresetName(FSCamera *camera) const
{
    if (!camera)
        return QString();

    FSCameraPathPresetsUmap::const_iterator iterator = d->umapPresetsLockCameras.find(camera->devicePath());

    if (iterator != d->umapPresetsLockCameras.end()) {
        return iterator->second;
    }

    return QString();
}

void FSLockPropertiesManager::setLockPropertiesInterval(int msec)
{
    d->sendLockedValuesTimer->setInterval(msec);
}

int FSLockPropertiesManager::lockPropertiesInterval() const
{
    return d->sendLockedValuesTimer->interval();
}

void FSLockPropertiesManager::loadLockSettings()
{
    // Manual lock properties
    {
        const QVariantMap varMap = FSSettings::lockProperties();
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

        d->umapManualLockCamerasParams = umapCameraPathValues;
    }

    // Lock presets
    {
        const QVariantMap varMap = FSSettings::lockPropertiesPresets();
        FSCameraPathPresetsUmap umapLockCamerasPresets;

        for (QVariantMap::const_iterator deviceIterator = varMap.begin();
             deviceIterator != varMap.end();
             ++deviceIterator) {
            const DevicePath &devicePath = deviceIterator.key();
            const QString &presetName = deviceIterator.value().toString();

            if (!devicePath.isEmpty() && !presetName.isEmpty()) {
                umapLockCamerasPresets.insert( { devicePath, presetName } );
            }
        }

        d->umapPresetsLockCameras = umapLockCamerasPresets;
    }
}

void FSLockPropertiesManager::saveLockSettings()
{
    // Manual lock properties
    {
        QVariantMap varMap;

        for (const auto&[devicePath, umapCameraPropertyValues] : d->umapManualLockCamerasParams) {
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

        FSSettings::setLockProperties(varMap);
    }

    // Lock presets
    {
        QVariantMap varMap;

        for (const auto&[devicePath, presetName] : d->umapPresetsLockCameras) {
            if (!devicePath.isEmpty() && !presetName.isEmpty()) {
                varMap.insert(devicePath, presetName);
            }
        }

        FSSettings::setLockPropertiesPresets(varMap);
    }

    FSSettings::sync();
}

bool FSLockPropertiesManager::isLockProperiesPause() const
{
    return (d->lockProperiesPauseTimer != nullptr);
}

void FSLockPropertiesManager::lockProperiesPause(int msec)
{
    if (isLockProperiesPause())
        lockProperiesPauseAbort();

    d->lockProperiesPauseTimer = new QTimer(this);
    d->lockProperiesPauseTimer->setSingleShot(true);
    d->lockProperiesPauseTimer->setInterval(msec);
    connect(d->lockProperiesPauseTimer, &QTimer::timeout,
            this,                       &FSLockPropertiesManager::lockProperiesPauseTimeout);
    d->lockProperiesPauseTimer->start();

    if (d->isLockPropertiesEnabled) {
        d->isLockPropertiesEnabled = false;
        updateTimerStatus();
        emit lockPropertiesEnableChanged(false);
    }
}

void FSLockPropertiesManager::lockProperiesPauseAbort()
{
    if (d->lockProperiesPauseTimer) {
        delete d->lockProperiesPauseTimer;
        d->lockProperiesPauseTimer = nullptr;
    }
}

void FSLockPropertiesManager::lockProperiesPauseFinish()
{
    lockProperiesPauseAbort();
    setLockPropertiesEnable(true);
}

bool FSLockPropertiesManager::manualLockProperty(FSCamera *camera,
                                                 FSCameraProperty property)
{
    if (!camera || isLockedProperty(camera, property))
        return false;

    FSValueParams valueParams;
    if (!camera->get(property, valueParams))
        return false;

    trySwitchToManualMode(camera);

    d->manualLockProperty(this, camera->devicePath(), property, valueParams);

    return true;
}

bool FSLockPropertiesManager::manualLockProperties(FSCamera *camera,
                                                   const std::vector<FSCameraProperty> &vecProperties)
{
    if (!camera)
        return false;

    for (const FSCameraProperty &property : vecProperties)
        if (!manualLockProperty(camera, property))
            return false;

    return true;
}

void FSLockPropertiesManager::manualLockProperty(FSCamera *camera,
                                                 FSCameraProperty property,
                                                 const FSValueParams &valueParams)
{
    if (!camera || isLockedProperty(camera, property))
        return;

    trySwitchToManualMode(camera);

    d->manualLockProperty(this, camera->devicePath(), property, valueParams);
}

void FSLockPropertiesManager::manualUnlockProperty(FSCamera *camera,
                                                   FSCameraProperty property)
{
    if (!camera || !isLockedProperty(camera, property))
        return;

    trySwitchToManualMode(camera);

    d->manualUnlockProperty(this, camera->devicePath(), property);
}

void FSLockPropertiesManager::manualUnlockProperties(FSCamera *camera,
                                                     const std::vector<FSCameraProperty> &vecProperties)
{
    if (!camera)
        return;

    for (const FSCameraProperty &property : vecProperties)
        manualUnlockProperty(camera, property);
}

void FSLockPropertiesManager::manualUnlockProperties(FSCamera *camera)
{
    if (!camera)
        return;

    FSCameraPathValuesUMap::const_iterator iterator = d->umapManualLockCamerasParams.find(camera->devicePath());

    if (iterator == d->umapManualLockCamerasParams.end())
        return;

    trySwitchToManualMode(camera);

    const FSCameraPropertyValuesUMap umapCameraPropertyValues = iterator->second;

    d->umapManualLockCamerasParams.erase(iterator);

    emit unlockedCamera(camera);

    for (const auto &[property, valueParams]: umapCameraPropertyValues) {
        emit unlockedProperty(camera, property);
    }

    updateTimerStatus();
}

void FSLockPropertiesManager::presetLockProperties(FSCamera *camera, const QString &presetName)
{
    if (!camera || !d->camerasStorage || presetName.isEmpty())
        return;

    FSCameraPathPresetsUmap::const_iterator presetIterator = d->umapPresetsLockCameras.find(camera->devicePath());
    if (presetIterator != d->umapPresetsLockCameras.end()) {
        if (presetIterator->second == presetName)
            return;

        presetUnlockProperies(camera);
    }

    d->umapPresetsLockCameras.insert( { camera->devicePath(), presetName } );
    emit lockedPreset(camera, presetName);

    const FSCameraPropertyValuesUMap umapCameraPresetPropertyValues = d->camerasStorage->getCameraUserPreset(camera->devicePath(), presetName);

    FSCameraPathValuesUMap::const_iterator manualIterator = d->umapManualLockCamerasParams.find(camera->devicePath());
    if (manualIterator == d->umapManualLockCamerasParams.end()) {
        emit lockedCamera(camera);

        for (const auto &[property, valueParams] : umapCameraPresetPropertyValues) {
            emit lockedProperty(camera, property);
        }
    } else {
        const FSCameraPropertyValuesUMap &umapCameraManualPropertyValues = manualIterator->second;

        for (const auto &[property, valueParams] : umapCameraPresetPropertyValues) {
            if (umapCameraManualPropertyValues.find(property) == umapCameraManualPropertyValues.end()) {
                emit lockedProperty(camera, property);
            }
        }

        for (const auto &[property, valueParams] : umapCameraManualPropertyValues) {
            if (umapCameraPresetPropertyValues.find(property) == umapCameraPresetPropertyValues.end()) {
                emit unlockedProperty(camera, property);
            }
        }
    }

    updateTimerStatus();
}

void FSLockPropertiesManager::presetUnlockProperies(FSCamera *camera)
{
    if (!camera || !d->camerasStorage)
        return;

    FSCameraPathPresetsUmap::const_iterator iterator = d->umapPresetsLockCameras.find(camera->devicePath());
    if (iterator == d->umapPresetsLockCameras.end())
        return;

    const QString oldPresetName = iterator->second;
    d->umapPresetsLockCameras.erase(iterator);
    emit unlockedPreset(camera);

    const FSCameraPropertyValuesUMap umapCameraPresetPropertyValues = d->camerasStorage->getCameraUserPreset(camera->devicePath(), oldPresetName);

    FSCameraPathValuesUMap::const_iterator manualIterator = d->umapManualLockCamerasParams.find(camera->devicePath());
    if (manualIterator == d->umapManualLockCamerasParams.end()) {
        for (const auto &[property, valueParams] : umapCameraPresetPropertyValues) {
            emit unlockedProperty(camera, property);
        }

        emit unlockedCamera(camera);
    } else {
        const FSCameraPropertyValuesUMap &umapCameraManualPropertyValues = manualIterator->second;

        for (const auto &[property, valueParams] : umapCameraPresetPropertyValues) {
            if (umapCameraManualPropertyValues.find(property) == umapCameraManualPropertyValues.end()) {
                emit unlockedProperty(camera, property);
            }
        }

        for (const auto &[property, valueParams] : umapCameraManualPropertyValues) {
            if (umapCameraPresetPropertyValues.find(property) == umapCameraPresetPropertyValues.end()) {
                emit lockedProperty(camera, property);
            }
        }
    }

    updateTimerStatus();
}

void FSLockPropertiesManager::init()
{
    d = new FSLockPropertiesManagerPrivate();

    d->sendLockedValuesTimer = new QTimer(this);
    d->sendLockedValuesTimer->setInterval(FSSettings::lockPropertiesInterval());
    connect(d->sendLockedValuesTimer, SIGNAL(timeout()),
            this,                       SLOT(sendAllLockedValues()));
    updateTimerStatus();
}

void FSLockPropertiesManager::updateTimerStatus()
{
    if (d->isLockPropertiesEnabled) {
        if (!isExistLockedProperties()) {
            if (d->sendLockedValuesTimer->isActive()) {
                d->sendLockedValuesTimer->stop();
            }
        } else {
            if (!d->sendLockedValuesTimer->isActive()) {
                d->sendLockedValuesTimer->start();
            }
        }
    } else {
        if (d->sendLockedValuesTimer->isActive()) {
            d->sendLockedValuesTimer->stop();
        }
    }
}

void FSLockPropertiesManager::copyCurrentPresetToManualPropertyValues(const DevicePath &devicePath)
{
    if (!d->camerasStorage)
        return;

    FSCameraPathPresetsUmap::const_iterator presetCameraIterator = d->umapPresetsLockCameras.find(devicePath);

    if (presetCameraIterator == d->umapPresetsLockCameras.end())
        return;

    d->umapPresetsLockCameras.erase(presetCameraIterator);

    const FSCameraPropertyValuesUMap presetPropertiesValues = d->camerasStorage->getCameraUserPreset(devicePath, presetCameraIterator->second);

    if (presetPropertiesValues.empty())
        return;

    FSCameraPathValuesUMap::iterator manualCameraIterator = d->umapManualLockCamerasParams.find(devicePath);

    if (manualCameraIterator != d->umapManualLockCamerasParams.end()) {
        manualCameraIterator->second = presetPropertiesValues;
    } else {
        d->umapManualLockCamerasParams.insert( { devicePath, presetPropertiesValues } );
    }
}

void FSLockPropertiesManager::trySwitchToManualMode(FSCamera *camera)
{
    if (d->umapPresetsLockCameras.find(camera->devicePath()) != d->umapPresetsLockCameras.end())
        switchToManualMode(camera);
}

void FSLockPropertiesManager::switchToManualMode(FSCamera *camera)
{
    copyCurrentPresetToManualPropertyValues(camera->devicePath());

    emit switchedToManualMode(camera);
}

bool FSLockPropertiesManager::setLockValueParams(FSCamera *camera,
                                                 FSCameraProperty property,
                                                 const FSValueParams &lockedValueParams)
{
    FSValueParams currentValueParams;

    if (camera->get(property, currentValueParams)) {
        if (currentValueParams != lockedValueParams) {
            if (!camera->set(property, lockedValueParams)) {
                qCritical("%s: Failed to set locked values(value:\"%ld\", flags:\"%ld\") for property \"%s\", cameraDevicePath=\"%s\"!",
                          metaObject()->className(),
                          lockedValueParams.value(),
                          lockedValueParams.flags(),
                          fsGetEnumName(property).toLocal8Bit().constData(),
                          camera->devicePath().toLocal8Bit().constData());
            } else {
                return true;
            }
        }
    } else {
        qCritical("%s: Failed to get values for property \"%s\", cameraDevicePath=\"%s\"!",
                  metaObject()->className(),
                  fsGetEnumName(property).toLocal8Bit().constData(),
                  camera->devicePath().toLocal8Bit().constData());
    }

    return false;
}

void FSLockPropertiesManager::addedCamera(const DevicePath &devicePath)
{
    if (!d->camerasStorage)
        return;

    FSCamera *camera = d->camerasStorage->findCameraByDevicePath(devicePath);

    if (!camera)
        return;

    FSCameraPathValuesUMap::const_iterator iterator = d->umapManualLockCamerasParams.find(devicePath);
    if (iterator != d->umapManualLockCamerasParams.end()) {
        emit lockedCamera(camera);

        const FSCameraPropertyValuesUMap &umapCameraPropertyValues = iterator->second;
        for (const auto&[property, value] : umapCameraPropertyValues) {
            emit lockedProperty(camera, property);
        }
    }
}

void FSLockPropertiesManager::removedCamera(const DevicePath &devicePath)
{
    if (!d->camerasStorage)
        return;

    FSCamera *camera = d->camerasStorage->findCameraByDevicePath(devicePath);

    if (!camera)
        return;

    FSCameraPathValuesUMap::const_iterator lockPropertiesIterator = d->umapManualLockCamerasParams.find(devicePath);
    if (lockPropertiesIterator != d->umapManualLockCamerasParams.end()) {
        emit unlockedCamera(camera);

        const FSCameraPropertyValuesUMap &umapCameraPropertyValues = lockPropertiesIterator->second;
        for (const auto&[property, value] : umapCameraPropertyValues) {
            emit unlockedProperty(camera, property);
        }
    }
}

void FSLockPropertiesManager::updateCurrentPreset(const DevicePath &devicePath)
{
    if (!d->camerasStorage)
        return;

    FSCameraPathPresetsUmap::const_iterator iterator = d->umapPresetsLockCameras.find(devicePath);

    if (iterator == d->umapPresetsLockCameras.end())
        return;

    const QString &lockPresetName = iterator->second;

    if (lockPresetName.isEmpty())
        return;

    if (!d->camerasStorage->isCameraUserPresetUsed(devicePath, lockPresetName))
        presetUnlockProperies(d->camerasStorage->findCameraByDevicePath(devicePath));
}

void FSLockPropertiesManager::lockProperiesPauseTimeout()
{
    lockProperiesPauseFinish();
}

void FSLockPropertiesManager::sendAllLockedValues()
{
    if (!d->camerasStorage)
        return;

    for (const auto &[devicePath, presetName] : d->umapPresetsLockCameras) {
        FSCamera *camera = d->camerasStorage->findCameraByDevicePath(devicePath);

        if (!camera || camera->isBlackListed())
            continue;

        const FSCameraPropertyValuesUMap presetPropertiesValues = d->camerasStorage->getCameraUserPreset(devicePath, presetName);
        for (const auto &[property, lockedValueParams] : presetPropertiesValues) {
            setLockValueParams(camera, property, lockedValueParams);
        }
    }

    for (const auto &[devicePath, lockedPropertiesValues] : d->umapManualLockCamerasParams) {
        FSCamera *camera = d->camerasStorage->findCameraByDevicePath(devicePath);

        if ( !camera ||
             camera->isBlackListed() ||
             d->umapPresetsLockCameras.find(devicePath) != d->umapPresetsLockCameras.end() )
            continue;

        for (const auto &[property, lockedValueParams] : lockedPropertiesValues) {
            setLockValueParams(camera, property, lockedValueParams);
        }
    }
}
