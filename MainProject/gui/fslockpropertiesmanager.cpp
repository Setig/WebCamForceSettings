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

#include <QMutex>
#include <QTimer>
#include <QThread>
#include <QSettings>

#include <WebCamFS/Settings>
#include <WebCamFS/CamerasStorage>

typedef std::unordered_map<DevicePath, QString> FSCameraPathPresetsUmap;

class FSLockPropertiesManagerPrivate
{
public:
    FSLockPropertiesManagerPrivate();

    QMutex camerasStorageMutex;
    FSCamerasStorage *camerasStorage;

    bool isLockPropertiesEnabled;

    QThread *parallelThread;

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
    , parallelThread(nullptr)
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

    FSCameraPathValuesUMap::iterator lockPropertiesIterator = umapManualLockCamerasParams.find(devicePath);

    if (lockPropertiesIterator == umapManualLockCamerasParams.end()) {
        FSCameraPropertyValuesUMap lockPropertiesUMapParams;
        lockPropertiesUMapParams.reserve(FSSettings::maxLockProperties());

        umapManualLockCamerasParams.insert( { devicePath, lockPropertiesUMapParams } );

        emit manager->lockedCamera(devicePath);

        lockPropertiesIterator = umapManualLockCamerasParams.find(devicePath);
    }

    FSCameraPropertyValuesUMap &umapCameraPropertyValues = lockPropertiesIterator->second;
    umapCameraPropertyValues.insert_or_assign(property, valueParams);

    emit manager->lockedProperty(devicePath, property);

    manager->updateTimerStatus();
}

void FSLockPropertiesManagerPrivate::manualUnlockProperty(FSLockPropertiesManager *manager,
                                                          const DevicePath &devicePath,
                                                          FSCameraProperty property)
{
    if (!camerasStorage)
        return;

    FSCameraPathValuesUMap::iterator iterator = umapManualLockCamerasParams.find(devicePath);
    if (iterator == umapManualLockCamerasParams.end())
        return;

    FSCameraPropertyValuesUMap &umapCameraPropertyValues = iterator->second;
    umapCameraPropertyValues.erase(property);

    if (umapCameraPropertyValues.empty()) {
        umapManualLockCamerasParams.erase(iterator);
        emit manager->unlockedCamera(devicePath);
    }

    manager->restoreDefaultPropertyValue(devicePath, property);

    emit manager->unlockedProperty(devicePath, property);

    manager->updateTimerStatus();
}

FSLockPropertiesManager::FSLockPropertiesManager(QObject *parent) : QObject(parent)
{
    init();
}

FSLockPropertiesManager::~FSLockPropertiesManager()
{
    QMetaObject::invokeMethod(d->sendLockedValuesTimer, &QTimer::stop, Qt::BlockingQueuedConnection);
    d->parallelThread->quit();
    d->parallelThread->wait();
    delete d->sendLockedValuesTimer;
    delete d->parallelThread;

    if (d->lockProperiesPauseTimer)
        delete d->lockProperiesPauseTimer;

    // Update statistics param
    {
        uint count = 1;

        for (const auto &[camera, lockedPropertiesValues] : d->umapManualLockCamerasParams) {
            uint size = uint(lockedPropertiesValues.size());

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
    QMutexLocker(&d->camerasStorageMutex);

    if (d->camerasStorage != camerasStorage) {
        if (d->camerasStorage) {
            disconnect(d->camerasStorage, &FSCamerasStorage::addedCamera,
                       this,              &FSLockPropertiesManager::addCamera);
            disconnect(d->camerasStorage, &FSCamerasStorage::removedCamera,
                       this,              &FSLockPropertiesManager::removeCamera);

            disconnect(d->camerasStorage, &FSCamerasStorage::cameraUserPresetNamesChanged,
                       this,              &FSLockPropertiesManager::updateCurrentPreset);
        }

        d->camerasStorage = camerasStorage;

        if (d->camerasStorage) {
            connect(d->camerasStorage, &FSCamerasStorage::addedCamera,
                    this,              &FSLockPropertiesManager::addCamera);
            connect(d->camerasStorage, &FSCamerasStorage::removedCamera,
                    this,              &FSLockPropertiesManager::removeCamera);

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

bool FSLockPropertiesManager::isLockedLeastOneProperty(const DevicePath &devicePath) const
{
    return (d->umapPresetsLockCameras.find(devicePath) != d->umapPresetsLockCameras.end() ||
            d->umapManualLockCamerasParams.find(devicePath) != d->umapManualLockCamerasParams.end());
}

bool FSLockPropertiesManager::isLockedProperty(const DevicePath &devicePath, FSCameraProperty property) const
{
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

FSValueParams FSLockPropertiesManager::lockedValueParams(const DevicePath &devicePath, FSCameraProperty property) const
{
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

bool FSLockPropertiesManager::isContaintsManualLockProperties(const DevicePath &devicePath) const
{
    return (d->umapManualLockCamerasParams.find(devicePath) != d->umapManualLockCamerasParams.end());
}

bool FSLockPropertiesManager::isContaintsPresetLock(const DevicePath &devicePath) const
{
    return (d->umapPresetsLockCameras.find(devicePath) != d->umapPresetsLockCameras.end());
}

std::vector<DevicePath> FSLockPropertiesManager::lockedDevicePaths() const
{
    std::vector<DevicePath> result;
    result.reserve(d->umapManualLockCamerasParams.size() + d->umapPresetsLockCameras.size());

    for (const auto &[devicePath, umapCameraPropertyValues] : d->umapManualLockCamerasParams) {
        result.push_back(devicePath);
    }

    for (const auto&[devicePath, presetName] : d->umapPresetsLockCameras) {
        if (d->umapManualLockCamerasParams.find(devicePath) == d->umapManualLockCamerasParams.end()) {
            result.push_back(devicePath);
        }
    }

    return result;
}

FSCameraPropertyValuesUMap FSLockPropertiesManager::lockedProperties(const DevicePath &devicePath) const
{
    FSCameraPropertyValuesUMap result;

    FSCameraPathValuesUMap::iterator iterator = d->umapManualLockCamerasParams.find(devicePath);
    if (iterator != d->umapManualLockCamerasParams.end()) {
        result = iterator->second;
    }

    return result;
}

FSLockPropertiesManager::LockMode FSLockPropertiesManager::currentLockMode(const DevicePath &devicePath) const
{
    if (d->umapPresetsLockCameras.find(devicePath) != d->umapPresetsLockCameras.end()) {
        return FSLockPropertiesManager::PresetLockMode;
    }

    if (d->umapManualLockCamerasParams.find(devicePath) != d->umapManualLockCamerasParams.end()) {
        return FSLockPropertiesManager::ManualLockMode;
    }

    return FSLockPropertiesManager::NoneLockMode;
}

QString FSLockPropertiesManager::lockPresetName(const DevicePath &devicePath) const
{
    FSCameraPathPresetsUmap::const_iterator iterator = d->umapPresetsLockCameras.find(devicePath);
    if (iterator != d->umapPresetsLockCameras.end()) {
        return iterator->second;
    }

    return QString();
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

    updateTimerStatus();
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

bool FSLockPropertiesManager::manualLockProperty(const DevicePath &devicePath,
                                                 FSCameraProperty property)
{
    if (isLockedProperty(devicePath, property))
        return false;

    FSCamera *camera = d->camerasStorage->findCameraByDevicePath(devicePath);

    if (!camera)
        return false;

    FSValueParams valueParams;
    if (!camera->get(property, valueParams))
        return false;

    trySwitchToManualMode(devicePath);

    d->manualLockProperty(this, camera->devicePath(), property, valueParams);

    return true;
}

bool FSLockPropertiesManager::manualLockProperties(const DevicePath &devicePath,
                                                   const std::vector<FSCameraProperty> &vecProperties)
{
    for (const FSCameraProperty &property : vecProperties)
        if (!manualLockProperty(devicePath, property))
            return false;

    return true;
}

void FSLockPropertiesManager::manualLockProperty(const DevicePath &devicePath,
                                                 FSCameraProperty property,
                                                 const FSValueParams &valueParams)
{
    if (isLockedProperty(devicePath, property))
        return;

    trySwitchToManualMode(devicePath);

    d->manualLockProperty(this, devicePath, property, valueParams);
}

void FSLockPropertiesManager::manualUnlockProperty(const DevicePath &devicePath,
                                                   FSCameraProperty property)
{
    if (!isLockedProperty(devicePath, property))
        return;

    trySwitchToManualMode(devicePath);

    d->manualUnlockProperty(this, devicePath, property);
}

void FSLockPropertiesManager::manualUnlockProperties(const DevicePath &devicePath,
                                                     const std::vector<FSCameraProperty> &vecProperties)
{
    for (const FSCameraProperty &property : vecProperties)
        manualUnlockProperty(devicePath, property);
}

void FSLockPropertiesManager::manualUnlockProperties(const DevicePath &devicePath)
{
    FSCameraPathValuesUMap::const_iterator iterator = d->umapManualLockCamerasParams.find(devicePath);

    if (iterator == d->umapManualLockCamerasParams.end())
        return;

    trySwitchToManualMode(devicePath);

    const FSCameraPropertyValuesUMap umapCameraPropertyValues = iterator->second;

    d->umapManualLockCamerasParams.erase(iterator);

    emit unlockedCamera(devicePath);

    for (const auto &[property, valueParams]: umapCameraPropertyValues) {
        restoreDefaultPropertyValue(devicePath, property);
        emit unlockedProperty(devicePath, property);
    }

    updateTimerStatus();
}

void FSLockPropertiesManager::presetLockProperties(const DevicePath &devicePath,
                                                   const QString &presetName)
{
    if (!d->camerasStorage || presetName.isEmpty())
        return;

    FSCameraPathPresetsUmap::const_iterator presetIterator = d->umapPresetsLockCameras.find(devicePath);
    if (presetIterator != d->umapPresetsLockCameras.end()) {
        if (presetIterator->second == presetName)
            return;

        presetUnlockProperies(devicePath);
    }

    d->umapPresetsLockCameras.insert( { devicePath, presetName } );
    emit lockedPreset(devicePath, presetName);

    const FSCameraPropertyValuesUMap umapCameraPresetPropertyValues = d->camerasStorage->getCameraUserPreset(devicePath, presetName);

    FSCameraPathValuesUMap::const_iterator manualIterator = d->umapManualLockCamerasParams.find(devicePath);
    if (manualIterator == d->umapManualLockCamerasParams.end()) {
        emit lockedCamera(devicePath);

        for (const auto &[property, valueParams] : umapCameraPresetPropertyValues) {
            emit lockedProperty(devicePath, property);
        }
    } else {
        const FSCameraPropertyValuesUMap &umapCameraManualPropertyValues = manualIterator->second;

        for (const auto &[property, valueParams] : umapCameraPresetPropertyValues) {
            if (umapCameraManualPropertyValues.find(property) == umapCameraManualPropertyValues.end()) {
                emit lockedProperty(devicePath, property);
            }
        }

        for (const auto &[property, valueParams] : umapCameraManualPropertyValues) {
            if (umapCameraPresetPropertyValues.find(property) == umapCameraPresetPropertyValues.end()) {
                restoreDefaultPropertyValue(devicePath, property);
                emit unlockedProperty(devicePath, property);
            }
        }
    }

    updateTimerStatus();
}

void FSLockPropertiesManager::presetUnlockProperies(const DevicePath &devicePath)
{
    if (!d->camerasStorage)
        return;

    FSCameraPathPresetsUmap::const_iterator iterator = d->umapPresetsLockCameras.find(devicePath);
    if (iterator == d->umapPresetsLockCameras.end())
        return;

    const QString oldPresetName = iterator->second;
    d->umapPresetsLockCameras.erase(iterator);
    emit unlockedPreset(devicePath);

    const FSCameraPropertyValuesUMap umapCameraPresetPropertyValues = d->camerasStorage->getCameraUserPreset(devicePath, oldPresetName);

    FSCameraPathValuesUMap::const_iterator manualIterator = d->umapManualLockCamerasParams.find(devicePath);
    if (manualIterator == d->umapManualLockCamerasParams.end()) {
        for (const auto &[property, valueParams] : umapCameraPresetPropertyValues) {
            restoreDefaultPropertyValue(devicePath, property);
            emit unlockedProperty(devicePath, property);
        }

        emit unlockedCamera(devicePath);
    } else {
        const FSCameraPropertyValuesUMap &umapCameraManualPropertyValues = manualIterator->second;

        for (const auto &[property, valueParams] : umapCameraPresetPropertyValues) {
            if (umapCameraManualPropertyValues.find(property) == umapCameraManualPropertyValues.end()) {
                restoreDefaultPropertyValue(devicePath, property);
                emit unlockedProperty(devicePath, property);
            }
        }

        for (const auto &[property, valueParams] : umapCameraManualPropertyValues) {
            if (umapCameraPresetPropertyValues.find(property) == umapCameraPresetPropertyValues.end()) {
                emit lockedProperty(devicePath, property);
            }
        }
    }

    updateTimerStatus();
}

void FSLockPropertiesManager::updateTimerStatus()
{
    if (d->isLockPropertiesEnabled) {
        if (!isExistLockedProperties()) {
            QMetaObject::invokeMethod(d->sendLockedValuesTimer,
                                      &QTimer::stop,
                                      Qt::QueuedConnection);
        } else {
            QMetaObject::invokeMethod(d->sendLockedValuesTimer,
                                      "start",
                                      Qt::QueuedConnection,
                                      Q_ARG(int, FSSettings::lockPropertiesInterval()));
        }
    } else {
        QMetaObject::invokeMethod(d->sendLockedValuesTimer,
                                  &QTimer::stop,
                                  Qt::QueuedConnection);
    }
}

void FSLockPropertiesManager::init()
{
    d = new FSLockPropertiesManagerPrivate();

    d->parallelThread = new QThread();
    d->parallelThread->start();

    d->sendLockedValuesTimer = new QTimer();
    d->sendLockedValuesTimer->moveToThread(d->parallelThread);
    d->sendLockedValuesTimer->setInterval(FSSettings::lockPropertiesInterval());
    connect(d->sendLockedValuesTimer, SIGNAL(timeout()),
            this,                       SLOT(sendAllLockedValues()));
    updateTimerStatus();
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

void FSLockPropertiesManager::trySwitchToManualMode(const DevicePath &devicePath)
{
    if (d->umapPresetsLockCameras.find(devicePath) != d->umapPresetsLockCameras.end())
        switchToManualMode(devicePath);
}

void FSLockPropertiesManager::switchToManualMode(const DevicePath &devicePath)
{
    copyCurrentPresetToManualPropertyValues(devicePath);

    emit switchedToManualMode(devicePath);
}

bool FSLockPropertiesManager::setLockValueParams(const DevicePath &devicePath,
                                                 FSCameraProperty property,
                                                 const FSValueParams &lockedValueParams)
{
    QMutexLocker(d->camerasStorage->cameraRecursiveMutex());
    FSCamera *camera = d->camerasStorage->findCameraByDevicePath(devicePath);

    if (!camera)
        return false;

    FSValueParams currentValueParams;

    if (camera->get(property, currentValueParams)) {
        if (currentValueParams != lockedValueParams) {
            if (!camera->set(property, lockedValueParams)) {
                qCritical("%s: Failed to set locked values(value:\"%ld\", flags:\"%ld\") for property \"%s\", cameraDevicePath=\"%s\"!",
                          metaObject()->className(),
                          lockedValueParams.value(),
                          lockedValueParams.flags(),
                          fsGetEnumName(property).toLocal8Bit().constData(),
                          devicePath.toLocal8Bit().constData());
            } else {
                return true;
            }
        }
    } else {
        qCritical("%s: Failed to get values for property \"%s\", cameraDevicePath=\"%s\"!",
                  metaObject()->className(),
                  fsGetEnumName(property).toLocal8Bit().constData(),
                  devicePath.toLocal8Bit().constData());
    }

    return false;
}

void FSLockPropertiesManager::restoreDefaultPropertyValue(const DevicePath &devicePath,
                                                          FSCameraProperty property)
{
    if (!d->camerasStorage)
        return;

    FSCamera *camera = d->camerasStorage->findCameraByDevicePath(devicePath);

    if (!camera)
        return;

    FSValueParams valueParams;

    if (d->camerasStorage->isUserDefaultValueUsed(devicePath, property)) {
        valueParams = d->camerasStorage->getUserDefaultValue(devicePath, property);
    } else {
        FSRangeParams rangeParams;
        camera->getRange(property, rangeParams);

        if (!rangeParams.isNull()) {
            valueParams = FSValueParams(rangeParams.value(),
                                        rangeParams.flags());
        }
    }

    if (!valueParams.isNull()) {
        camera->set(property, valueParams);
    }
}

void FSLockPropertiesManager::addCamera(const DevicePath &devicePath)
{
    if (!d->camerasStorage)
        return;

    FSCameraPathValuesUMap::const_iterator iterator = d->umapManualLockCamerasParams.find(devicePath);
    if (iterator != d->umapManualLockCamerasParams.end()) {
        emit lockedCamera(devicePath);

        const FSCameraPropertyValuesUMap &umapCameraPropertyValues = iterator->second;
        for (const auto&[property, value] : umapCameraPropertyValues) {
            emit lockedProperty(devicePath, property);
        }
    }
}

void FSLockPropertiesManager::removeCamera(const DevicePath &devicePath)
{
    if (!d->camerasStorage)
        return;

    FSCameraPathValuesUMap::const_iterator lockPropertiesIterator = d->umapManualLockCamerasParams.find(devicePath);
    if (lockPropertiesIterator != d->umapManualLockCamerasParams.end()) {
        emit unlockedCamera(devicePath);

        const FSCameraPropertyValuesUMap &umapCameraPropertyValues = lockPropertiesIterator->second;
        for (const auto&[property, value] : umapCameraPropertyValues) {
            restoreDefaultPropertyValue(devicePath, property);
            emit unlockedProperty(devicePath, property);
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
        presetUnlockProperies(devicePath);
}

void FSLockPropertiesManager::lockProperiesPauseTimeout()
{
    lockProperiesPauseFinish();
}

void FSLockPropertiesManager::sendAllLockedValues()
{
    QMutexLocker(&d->camerasStorageMutex);

    if (!d->camerasStorage)
        return;

    for (const auto &[devicePath, presetName] : d->umapPresetsLockCameras) {
        if ( !d->camerasStorage->isCameraConnected(devicePath) ||
             d->camerasStorage->isContaintsBlackList(devicePath) )
            continue;

        const FSCameraPropertyValuesUMap presetPropertiesValues = d->camerasStorage->getCameraUserPreset(devicePath, presetName);
        for (const auto &[property, lockedValueParams] : presetPropertiesValues) {
            setLockValueParams(devicePath, property, lockedValueParams);
        }
    }

    for (const auto &[devicePath, lockedPropertiesValues] : d->umapManualLockCamerasParams) {
        if ( !d->camerasStorage->isCameraConnected(devicePath) ||
             d->camerasStorage->isContaintsBlackList(devicePath) ||
             d->umapPresetsLockCameras.find(devicePath) != d->umapPresetsLockCameras.end() )
            continue;

        for (const auto &[property, lockedValueParams] : lockedPropertiesValues) {
            setLockValueParams(devicePath, property, lockedValueParams);
        }
    }
}
