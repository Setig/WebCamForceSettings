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

#include "fssystemtrayicon.h"

#include <unordered_map>

#include <QMenu>
#include <QTimer>
#include <QThread>
#include <QMessageBox>
#include <QApplication>

#ifdef BUILD_WITH_Q_EASY_SETTINGS
#include "qeasysettings.hpp"
#endif // BUILD_WITH_Q_EASY_SETTINGS

#include <WebCamFS/Lib>
#include <WebCamFS/Settings>
#include <WebCamFS/AutoStart>
#include <WebCamFS/IconCreator>
#include <WebCamFS/CamerasStorage>
#include <WebCamFS/SettingsDialog>
#include <WebCamFS/TranslationsHelper>
#include <WebCamFS/CameraSettingsDialog>
#include <WebCamFS/LockPropertiesManager>
#include <WebCamFS/CameraUserSettingsDialog>

#define TR_MANUAL_TEXT FSSystemTrayIcon::tr("Manual")

typedef std::unordered_map<FSCamera *, QAction *> FSCamerasAndActionsUMap;
typedef std::unordered_map<QAction *, FSCamera *> FSActionsAndCamerasUMap;

typedef std::unordered_map<FSCamera *, QMenu *> FSCamerasAndMenuUMap;
typedef std::unordered_map<QMenu *, FSCamera *> FSMenuAndCamerasUMap;

typedef std::unordered_map<FSCamera *, FSCameraSettingsDialog *> FSCameraAndOpenedCameraSettingsDialogUMap;
typedef std::unordered_map<FSCameraSettingsDialog *, FSCamera *> FSOpenedCameraSettingsDialogAndCameraUMap;

class FSSystemTrayIconPrivate
{
public:
    FSSystemTrayIconPrivate();

    QMenu *mainMenu;

    QTimer *camerasDetectionTimer;

    QMenu *cameraSettingsMenu;

    QAction *lockPropertiesPauseBeforeSeparator;
    QMenu   *lockPropertiesPauseMenu;
    QAction *lockPropertiesPauseAbortAction;

    QThread *parallelThread;

    FSLockPropertiesManager *lockPropertiesManager;
    FSCamerasStorage *camerasStorage;

    // For fast search camera
    FSCamerasAndActionsUMap umapCamerasAndActions;
    FSActionsAndCamerasUMap umapActionsAndCameras;

    FSCamerasAndMenuUMap umapCamerasAndPresetMenu;
    FSMenuAndCamerasUMap umapPresetMenuAndCameras;

    FSCameraAndOpenedCameraSettingsDialogUMap umapCameraAndOpenedCameraSettingsDialog;
    FSOpenedCameraSettingsDialogAndCameraUMap umapOpenedCameraSettingsDialogAndCamera;

    FSSettingsDialog *settingsDialog;
    FSCameraUserSettingsDialog *cameraSettingsDialog;
};

FSSystemTrayIconPrivate::FSSystemTrayIconPrivate()
    : mainMenu(nullptr)
    , camerasDetectionTimer(nullptr)
    , cameraSettingsMenu(nullptr)
    , lockPropertiesPauseBeforeSeparator(nullptr)
    , lockPropertiesPauseMenu(nullptr)
    , lockPropertiesPauseAbortAction(nullptr)
    , parallelThread(nullptr)
    , lockPropertiesManager(nullptr)
    , camerasStorage(nullptr)
    , settingsDialog(nullptr)
    , cameraSettingsDialog(nullptr)
{
    const size_t standartCamerasCount = FSSettings::usuallyAvailableCamerasCount();

    umapCamerasAndActions.reserve(standartCamerasCount);
    umapActionsAndCameras.reserve(standartCamerasCount);

    umapCamerasAndPresetMenu.reserve(standartCamerasCount);
    umapPresetMenuAndCameras.reserve(standartCamerasCount);

    umapCameraAndOpenedCameraSettingsDialog.reserve(standartCamerasCount);
    umapOpenedCameraSettingsDialogAndCamera.reserve(standartCamerasCount);
}

FSSystemTrayIcon::FSSystemTrayIcon(QObject *parent)
    : QSystemTrayIcon(parent)
{
    init();
}

FSSystemTrayIcon::~FSSystemTrayIcon()
{
    hide();

    d->camerasStorage->saveAll();

    QMetaObject::invokeMethod(d->camerasDetectionTimer, &QTimer::stop, Qt::BlockingQueuedConnection);
    d->parallelThread->quit();
    d->parallelThread->wait();
    delete d->camerasDetectionTimer;
    delete d->parallelThread;

    for (const auto &[camera, cameraSettingsDialog] : d->umapCameraAndOpenedCameraSettingsDialog) {
        delete cameraSettingsDialog;
    }

    delete d->settingsDialog;
    delete d->cameraSettingsDialog;

    if (d->lockPropertiesManager)
        d->lockPropertiesManager->saveLockSettings();

    FSCamera::uninitializeWinCOMLibrary();

    delete d->cameraSettingsMenu;
    delete d->lockPropertiesManager;
    delete d->camerasStorage;

    FSSettings::uninitialization();
    FSTranslationsHelper::uninitialization();

    delete d;
    d = nullptr;
}

void FSSystemTrayIcon::init()
{
    d = new FSSystemTrayIconPrivate();

    d->parallelThread = new QThread();
    d->parallelThread->start();

#ifdef BUILD_WITH_Q_EASY_SETTINGS
    QEasySettings::setStyle(QEasySettings::Style(FSSettings::customStyleIndex()));
#endif // BUILD_WITH_Q_EASY_SETTINGS

    setIcon(qApp->windowIcon());

    connect(fsTH, &FSTranslationsHelper::currentLanguageChanged,
            this, &FSSystemTrayIcon::retranslate);

    d->camerasStorage = new FSCamerasStorage(this);
    d->camerasStorage->loadAll();
    connect(d->camerasStorage, &FSCamerasStorage::cameraUserNameChanged,
            this,              &FSSystemTrayIcon::updateDisplayName);
    connect(d->camerasStorage, &FSCamerasStorage::addedBlacklistCamera,
            this,              &FSSystemTrayIcon::updateActionsByBlackList);
    connect(d->camerasStorage, &FSCamerasStorage::removedBlacklistCamera,
            this,              &FSSystemTrayIcon::updateActionsByBlackList);
    connect(d->camerasStorage, &FSCamerasStorage::cameraUserPresetNamesChanged,
            this,              &FSSystemTrayIcon::updatePresetsMenuByDevicePath);

    d->camerasDetectionTimer = new QTimer();
    d->camerasDetectionTimer->moveToThread(d->parallelThread);
    connect(d->camerasDetectionTimer, SIGNAL(timeout()),
            this,                       SLOT(checkAvailableCameras()));
    updateCamerasDetectionTimer();

    updateLockPropertiesManager();

    initMainMenu();

    QTimer::singleShot(0, this, &FSSystemTrayIcon::checkAvailableCameras);
}

void FSSystemTrayIcon::initMainMenu()
{
    d->mainMenu = new QMenu(tr("Main menu"), nullptr);

    d->mainMenu->addAction(tr("About..."), this, SLOT(about()));
    d->mainMenu->addAction(tr("About Qt..."), qApp, SLOT(aboutQt()));
    d->mainMenu->addSeparator();
    d->cameraSettingsMenu = new QMenu(tr("Camera settings"), d->mainMenu);
    connect(d->cameraSettingsMenu, &QMenu::aboutToShow,
            this,                  &FSSystemTrayIcon::onShowAvailableCamerasMenu);
    d->mainMenu->addMenu(d->cameraSettingsMenu);
    d->mainMenu->addSeparator();
    d->mainMenu->addAction(tr("Application settings..."), this, SLOT(showSettingsDialog()));
    d->mainMenu->addAction(tr("User settings..."),        this, SLOT(showUserSettingsDialog()));
    d->mainMenu->addSeparator();
    d->mainMenu->addAction(tr("Exit"), qApp, SLOT(quit()));
    setContextMenu(d->mainMenu);

    setToolTip(tr("WebCam force settings"));
}

void FSSystemTrayIcon::createCameraMenuObjects(FSCamera *camera)
{
    QAction *cameraAction = new QAction(camera->displayName(), d->cameraSettingsMenu);
    cameraAction->setVisible(!d->camerasStorage->isContaintsBlackList(camera->devicePath()));
    connect(cameraAction, SIGNAL(triggered(bool)),
            this,           SLOT(showCameraSettingsDialog()));

    d->cameraSettingsMenu->insertAction(beforeActionForListActions(d->cameraSettingsMenu->actions(),
                                                                   cameraAction->text()),
                                        cameraAction);

    d->umapCamerasAndActions.insert( { camera, cameraAction } );
    d->umapActionsAndCameras.insert( { cameraAction, camera });

    std::vector<QString> vectorPresetNames = d->camerasStorage->getCameraUserPresetNames(camera->devicePath());
    if (!vectorPresetNames.empty()) {
        QMenu *presetMenu = createPresetMenu(camera, cameraAction);
        updatePresetsMenu(camera, presetMenu, vectorPresetNames);
    }
}

void FSSystemTrayIcon::deleteCameraMenuObjects(FSCamera *camera)
{
    FSCamerasAndActionsUMap::const_iterator actionIterator = d->umapCamerasAndActions.find(camera);
    if (actionIterator != d->umapCamerasAndActions.end()) {
        QAction *cameraAction = actionIterator->second;
        d->cameraSettingsMenu->removeAction(cameraAction);
        delete cameraAction;

        d->umapCamerasAndActions.erase(actionIterator);
        d->umapActionsAndCameras.erase(cameraAction);
    }

    FSCamerasAndMenuUMap::const_iterator presetMenuIterator = d->umapCamerasAndPresetMenu.find(camera);
    if (presetMenuIterator != d->umapCamerasAndPresetMenu.end()) {
        QMenu *presetMenu = presetMenuIterator->second;
        delete presetMenu;

        d->umapCamerasAndPresetMenu.erase(presetMenuIterator);
        d->umapPresetMenuAndCameras.erase(presetMenu);
    }

    FSCameraAndOpenedCameraSettingsDialogUMap::const_iterator dialogIterator = d->umapCameraAndOpenedCameraSettingsDialog.find(camera);
    if (dialogIterator != d->umapCameraAndOpenedCameraSettingsDialog.end()) {
        dialogIterator->second->close();
    }
}

QAction *FSSystemTrayIcon::beforeActionForListActions(const QList<QAction *> &listActions,
                                                      const QString &newActionText)
{
    foreach (QAction *action, listActions) {
        if (action->text() > newActionText) {
            return action;
        }
    }

    return nullptr;
}

void FSSystemTrayIcon::updateCamerasDetectionTimer()
{
    if (FSSettings::isCameraDetectionEnable()) {
        QMetaObject::invokeMethod(d->camerasDetectionTimer,
                                  "start",
                                  Qt::QueuedConnection,
                                  Q_ARG(int, FSSettings::cameraDetectionInterval()));
    } else {
        QMetaObject::invokeMethod(d->camerasDetectionTimer,
                                  &QTimer::stop,
                                  Qt::QueuedConnection);
    }
}

void FSSystemTrayIcon::updateSettingsDialogs()
{
    for (const auto &[camera, cameraSettingsDialog] : d->umapCameraAndOpenedCameraSettingsDialog) {
        cameraSettingsDialog->setCameraValueUpdateInterval(FSSettings::cameraValueUpdateInterval());
        cameraSettingsDialog->setCameraValueUpdateEnable(FSSettings::isCameraValueUpdateEnable());
    }
}

void FSSystemTrayIcon::updateLockPropertiesManager()
{
    if (FSSettings::isLockPropertiesEnable()) {
        if (!d->lockPropertiesManager) {
            d->lockPropertiesManager = new FSLockPropertiesManager();
            d->lockPropertiesManager->setCamerasStorage(d->camerasStorage);
            d->lockPropertiesManager->loadLockSettings();
            connect(d->lockPropertiesManager, SIGNAL(lockedCamera(DevicePath)),
                    this,                       SLOT(updatePresetsMenuByDevicePath(DevicePath)));
            connect(d->lockPropertiesManager, SIGNAL(unlockedCamera(DevicePath)),
                    this,                       SLOT(updatePresetsMenuByDevicePath(DevicePath)));
            connect(d->lockPropertiesManager, SIGNAL(switchedToManualMode(DevicePath)),
                    this,                       SLOT(updatePresetsMenuByDevicePath(DevicePath)));
            connect(d->lockPropertiesManager, SIGNAL(lockedPreset(DevicePath,QString)),
                    this,                       SLOT(updatePresetsMenuByDevicePath(DevicePath)));
            connect(d->lockPropertiesManager, SIGNAL(unlockedPreset(DevicePath)),
                    this,                       SLOT(updatePresetsMenuByDevicePath(DevicePath)));

            connect(d->lockPropertiesManager, SIGNAL(lockedCamera(DevicePath)),
                    this,                       SLOT(updateLockPropertiesPauseMenu()));
            connect(d->lockPropertiesManager, SIGNAL(unlockedCamera(DevicePath)),
                    this,                       SLOT(updateLockPropertiesPauseMenu()));
            connect(d->lockPropertiesManager, SIGNAL(lockPropertiesEnableChanged(bool)),
                    this,                       SLOT(updateLockPropertiesPauseMenu()));

            for (const auto &[camera, cameraSettingsDialog] : d->umapCameraAndOpenedCameraSettingsDialog) {
                cameraSettingsDialog->setLockPropertiesManager(d->lockPropertiesManager);
            }

            if (d->cameraSettingsDialog)
                d->cameraSettingsDialog->setLockPropertiesManager(d->lockPropertiesManager);
        } else {
            d->lockPropertiesManager->updateTimerStatus();
        }
    } else {
        if (d->lockPropertiesManager) {
            for (const auto &[camera, cameraSettingsDialog] : d->umapCameraAndOpenedCameraSettingsDialog) {
                cameraSettingsDialog->setLockPropertiesManager(nullptr);
            }

            if (d->cameraSettingsDialog)
                d->cameraSettingsDialog->setLockPropertiesManager(nullptr);

            d->lockPropertiesManager->saveLockSettings();

            delete d->lockPropertiesManager;
            d->lockPropertiesManager = nullptr;

            updateLockPropertiesPauseMenu();
        }
    }
}

void FSSystemTrayIcon::showCameraSettingsDialog(FSCamera *camera)
{
    if (!camera)
        return;

    FSCameraAndOpenedCameraSettingsDialogUMap::const_iterator dialogIterator = d->umapCameraAndOpenedCameraSettingsDialog.find(camera);

    if (dialogIterator != d->umapCameraAndOpenedCameraSettingsDialog.end()) {
        FSCameraSettingsDialog *existDialog = dialogIterator->second;
        existDialog->show();
        existDialog->raise();
        existDialog->activateWindow();
    } else {
        FSCameraSettingsDialog *dialog = new FSCameraSettingsDialog(camera);
        dialog->setAttribute(Qt::WA_DeleteOnClose, true);
        dialog->setMode(FSCameraSettingsDialog::LockPropertiesMode);
        dialog->setLockPropertiesManager(d->lockPropertiesManager);

        connect(dialog, SIGNAL(destroyed(QObject*)),
                this,     SLOT(removeOpenedCameraSettingsDialog(QObject*)));

        dialog->show();

        d->umapCameraAndOpenedCameraSettingsDialog.insert( { camera, dialog } );
        d->umapOpenedCameraSettingsDialogAndCamera.insert( { dialog, camera } );
    }
}

QMenu *FSSystemTrayIcon::createPresetMenu(FSCamera *camera, QAction *cameraAction)
{
    QMenu *presetMenu = new QMenu(d->cameraSettingsMenu);
    cameraAction->setMenu(presetMenu);

    QAction *action = new QAction(tr("Show current settings"), presetMenu);
    connect(action, SIGNAL(triggered(bool)),
            this,     SLOT(showCameraSettingsDialog()));
    presetMenu->addAction(action);

    d->umapCamerasAndPresetMenu.insert( { camera, presetMenu } );
    d->umapPresetMenuAndCameras.insert( { presetMenu, camera } );

    return presetMenu;
}

void FSSystemTrayIcon::updatePresetsMenu(FSCamera *camera,
                                         QMenu *presetMenu,
                                         const std::vector<QString> &presetNames)
{
    if (!presetMenu || !camera)
        return;

    // Remove old preset menu objects
    {
        QList<QAction *> listAction = presetMenu->actions();

        if (!listAction.isEmpty()) {
            // Ignore first action (show current settings)
            listAction.removeFirst();

            QActionGroup *presetActionGroup = nullptr;

            foreach (QAction *action, listAction) {
                QActionGroup *tmpPresetActionGroup = action->actionGroup();

                presetMenu->removeAction(action);

                if (presetActionGroup && presetActionGroup != tmpPresetActionGroup) {
                    delete presetActionGroup;
                }

                presetActionGroup = tmpPresetActionGroup;

                delete action;
            }

            delete presetActionGroup;
        }
    }

    FSLockPropertiesManager *lockPropertiesManager = d->lockPropertiesManager;

    QAction *manualModeAction = nullptr;
    if (lockPropertiesManager && lockPropertiesManager->isContaintsManualLockProperties(camera->devicePath())) {
        presetMenu->addSeparator();
        manualModeAction = presetMenu->addAction(TR_MANUAL_TEXT);

        connect(manualModeAction, &QAction::triggered,
                this,             &FSSystemTrayIcon::switchCameraPreset);

        manualModeAction->setCheckable(true);

        if (!lockPropertiesManager->isContaintsPresetLock(camera->devicePath()))
            manualModeAction->setChecked(true);
    }

    FSMenuAndCamerasUMap::const_iterator iterator = d->umapPresetMenuAndCameras.find(presetMenu);
    if (iterator != d->umapPresetMenuAndCameras.end()) {
        if (!presetNames.empty()) {
            // Create action group for preset menu (The action group will be deleted when the preset menu is deleted)
            QActionGroup *presetActionGroup = new QActionGroup(presetMenu);

            QString lockPresetName;
            if (lockPropertiesManager) {
                if (lockPropertiesManager->isContaintsManualLockProperties(camera->devicePath())) {
                    presetActionGroup->addAction(manualModeAction);
                } else {
                    presetMenu->addSeparator();
                }

                lockPresetName = lockPropertiesManager->lockPresetName(camera->devicePath());
            } else {
                presetMenu->addSeparator();
            }

            std::vector<QString> tmpPresetNames = presetNames;
            sort(tmpPresetNames.begin(), tmpPresetNames.end());

            for (const QString &presetName : tmpPresetNames) {
                QAction *presetAction = new QAction(presetName, presetMenu);
                presetAction->setActionGroup(presetActionGroup);
                connect(presetAction, &QAction::triggered,
                        this,         &FSSystemTrayIcon::switchCameraPreset);

                presetAction->setCheckable(true);

                if (!lockPresetName.isEmpty() && lockPresetName == presetName) {
                    presetAction->setChecked(true);
                }

                presetMenu->addAction(presetAction);
            }

            return;
        }
    }

    delete presetMenu;

    if (iterator != d->umapPresetMenuAndCameras.end()) {
        d->umapCamerasAndPresetMenu.erase(iterator->second);
        d->umapPresetMenuAndCameras.erase(iterator);
    }
}

void FSSystemTrayIcon::registerCamera(const FSCameraData &cameraData)
{
    const DeviceName freeDeviceName = d->camerasStorage->getFreeDeviceName(cameraData.name());
    const FSCameraData newCameraData(freeDeviceName, cameraData.devicePath(), cameraData.pCap());

    FSCamera *newCamera = new FSCamera(newCameraData);
    d->camerasStorage->registerCamera(newCamera);

    createCameraMenuObjects(newCamera);
}

void FSSystemTrayIcon::unregisterCamera(FSCamera *camera)
{
    d->camerasStorage->unregisterCamera(camera);
    deleteCameraMenuObjects(camera);
    delete camera;
}

void FSSystemTrayIcon::retranslate()
{
    delete d->mainMenu;
    d->mainMenu = nullptr;

    // Childs main menu
    d->cameraSettingsMenu = nullptr;
    d->lockPropertiesPauseBeforeSeparator = nullptr;
    d->lockPropertiesPauseMenu = nullptr;
    d->lockPropertiesPauseAbortAction = nullptr;
    d->umapCamerasAndActions.clear();
    d->umapActionsAndCameras.clear();
    d->umapCamerasAndPresetMenu.clear();
    d->umapPresetMenuAndCameras.clear();

    initMainMenu();
    updateLockPropertiesPauseMenu();

    const std::vector<FSCamera *> &availableCameras = d->camerasStorage->availableCameras();
    for (FSCamera *camera : availableCameras) {
        createCameraMenuObjects(camera);
    }
}

void FSSystemTrayIcon::about()
{
    QString translatedTextAboutCaption = tr("<h3>About %1</h3><p>Library version: %2</p><p>Application version: %3</p>")
            .arg(qApp->applicationName(),
                 fsLibraryVersion(),
                 qApp->applicationVersion());

    const QString translatedTextAboutText = tr(
                "<p>The application is designed to work with webcam settings:</p>"
                "<p> * change the settings of any webcam in the system while the camera is running;</p>"
                "<p> * block the properties of the webcam if a third-party application resets certain parameters;</p>"
                "<p> * switch between prepared webcam presets.</p>"
                "<p>Author by %3."
                ).arg("Alexander Tsyganov");

    QMessageBox msgBox(qApp->activeWindow());
    msgBox.setWindowTitle(tr("About %1...").arg(qApp->applicationName()));
    msgBox.setText(translatedTextAboutCaption);
    msgBox.setInformativeText(translatedTextAboutText);
    msgBox.setIconPixmap(FSIconCreator::generate(64));

    // Side projects about
    {
        QStringList projectsNames;

#ifdef BUILD_WITH_QT_SINGLE_APPLICATION
        projectsNames += "QtSingleApplication(Digia Plc and/or its subsidiary(-ies))";
#endif // BUILD_WITH_QT_SINGLE_APPLICATION

#ifdef BUILD_WITH_Q_EASY_SETTINGS
        projectsNames += "QEasySettings(mguludag)";
#endif // BUILD_WITH_Q_EASY_SETTINGS

        if (!projectsNames.isEmpty())
            msgBox.setDetailedText(tr("Used side projects: \n%1.").arg(projectsNames.join("\n")));
    }

    msgBox.exec();
}

void FSSystemTrayIcon::showSettingsDialog()
{
    if (d->settingsDialog) {
        d->settingsDialog->show();
        d->settingsDialog->raise();
        d->settingsDialog->activateWindow();
        return;
    }

    d->settingsDialog = new FSSettingsDialog();

    FSAutoStartLastError isAutoStartLastError;
    const bool isAutoStart = FSAutoStart::isAutoStart(&isAutoStartLastError);

    if (isAutoStartLastError.isSuccess()) {
        d->settingsDialog->setAutoStartEnable(true);
        d->settingsDialog->setAutoStart(isAutoStart);
    } else {
        qCritical("%s: Failed to get auto start state(step=%d, error=%ld)!",
                  FSSystemTrayIcon::staticMetaObject.className(),
                  isAutoStartLastError.step(),
                  isAutoStartLastError.error());

        d->settingsDialog->setAutoStartEnable(false);
        d->settingsDialog->setAutoStart(false);
    }

    fsTH->updateAvailableFSLocales();
    d->settingsDialog->setCurrentLocale(FSSettings::currentLocale());

#ifdef BUILD_WITH_Q_EASY_SETTINGS
    d->settingsDialog->setCustomStyleIndex(FSSettings::customStyleIndex());
#endif // BUILD_WITH_Q_EASY_SETTINGS

    d->settingsDialog->setCameraDetectionEnable(FSSettings::isCameraDetectionEnable());
    d->settingsDialog->setCameraDetectionInterval(FSSettings::cameraDetectionInterval());
    d->settingsDialog->setCameraValueUpdateEnable(FSSettings::isCameraValueUpdateEnable());
    d->settingsDialog->setCameraValueUpdateInterval(FSSettings::cameraValueUpdateInterval());
    d->settingsDialog->setLockPropertiesEnable(FSSettings::isLockPropertiesEnable());
    d->settingsDialog->setLockPropertiesInterval(FSSettings::lockPropertiesInterval());

    int res = d->settingsDialog->exec();

    if (res == QDialog::Accepted) {
        if (isAutoStartLastError.isSuccess()) {
            FSAutoStartLastError lastError;
            FSAutoStart::setAutoStart(d->settingsDialog->isAutoStart(), &lastError);

            if (!lastError.isSuccess()) {
                qCritical("%s: Failed to set auto start state(step=%d, error=%ld)!",
                          FSSystemTrayIcon::staticMetaObject.className(),
                          isAutoStartLastError.step(),
                          isAutoStartLastError.error());

                QMessageBox::critical(qApp->activeWindow(),
                                      tr("Set auto start error"),
                                      tr("Failed to change auto start state(step=%1, error=%2).").arg(isAutoStartLastError.step(),
                                                                                                      isAutoStartLastError.error()));
            }
        }

        FSSettings::setCurrentLocale(d->settingsDialog->currentLocale());
        fsTH->setCurrentLocale(d->settingsDialog->currentLocale());

#ifdef BUILD_WITH_Q_EASY_SETTINGS
        const int styleIndex = d->settingsDialog->customStyleIndex();
        FSSettings::setCustomStyleIndex(styleIndex);
        QEasySettings::setStyle(QEasySettings::Style(styleIndex));
#endif // BUILD_WITH_Q_EASY_SETTINGS

        FSSettings::setCameraDetectionEnable(d->settingsDialog->isCameraDetectionEnable());
        FSSettings::setCameraDetectionInterval(d->settingsDialog->cameraDetectionInterval());
        FSSettings::setCameraValueUpdateEnable(d->settingsDialog->isCameraValueUpdateEnable());
        FSSettings::setCameraValueUpdateInterval(d->settingsDialog->cameraValueUpdateInterval());
        FSSettings::setLockPropertiesEnable(d->settingsDialog->isLockPropertiesEnable());
        FSSettings::setLockPropertiesInterval(d->settingsDialog->lockPropertiesInterval());
        FSSettings::sync();

        updateCamerasDetectionTimer();
        updateSettingsDialogs();
        updateLockPropertiesManager();
    }

    delete d->settingsDialog;
    d->settingsDialog = nullptr;
}

void FSSystemTrayIcon::showUserSettingsDialog()
{
    if (d->cameraSettingsDialog) {
        d->cameraSettingsDialog->show();
        d->cameraSettingsDialog->raise();
        d->cameraSettingsDialog->activateWindow();
        return;
    }

    d->cameraSettingsDialog = new FSCameraUserSettingsDialog();
    d->cameraSettingsDialog->setAttribute(Qt::WA_DeleteOnClose, true);
    d->cameraSettingsDialog->setCamerasStorage(d->camerasStorage);
    d->cameraSettingsDialog->setLockPropertiesManager(d->lockPropertiesManager);

    connect(d->cameraSettingsDialog, SIGNAL(showCameraSettingsDialog(DevicePath)),
            this,                      SLOT(showCameraSettingsDialog(DevicePath)));

    connect(d->cameraSettingsDialog, SIGNAL(destroyed(QObject*)),
            this,                      SLOT(removeUserSettingsDialog(QObject*)));

    d->cameraSettingsDialog->show();

    connect(d->cameraSettingsDialog, SIGNAL(finished(int)),
            d->camerasStorage,         SLOT(saveAll()));
}

void FSSystemTrayIcon::onShowAvailableCamerasMenu()
{
    if (!FSSettings::isCameraDetectionEnable())
        checkAvailableCameras();
}

void FSSystemTrayIcon::checkAvailableCameras()
{
    FSCamerasDataUMap umapAvailableCamerasData = FSCamera::availableCamerasDataUMap();

    // Checking new cameras
    for (auto &[devicePath, cameraData] : umapAvailableCamerasData) {
        if (!d->camerasStorage->isCameraConnected(devicePath)) {
            QMetaObject::invokeMethod(this,
                                      "registerCamera",
                                      Qt::QueuedConnection,
                                      Q_ARG(FSCameraData, cameraData));
        } else {
            // Release cap pointer which will no longer be used.
            cameraData.release();
        }
    }

    // Checking removed cameras
    const FSCameraPathsUMap cameraPathsUMap = d->camerasStorage->getCameraPathUMap();
    for (const auto &[devicePath, camera] : cameraPathsUMap) {
        if (umapAvailableCamerasData.find(devicePath) == umapAvailableCamerasData.end()) {
            QMetaObject::invokeMethod(this,
                                      "unregisterCamera",
                                      Qt::QueuedConnection,
                                      Q_ARG(FSCamera*, camera));
        }
    }
}

void FSSystemTrayIcon::updateActionsByBlackList(const DevicePath &devicePath)
{
    FSCamera *camera = d->camerasStorage->findCameraByDevicePath(devicePath);

    if (!camera)
        return;

    FSCamerasAndActionsUMap::const_iterator iterator = d->umapCamerasAndActions.find(camera);

    if (iterator == d->umapCamerasAndActions.end())
        return;

    iterator->second->setVisible(!camera->isBlackListed());
}

void FSSystemTrayIcon::updatePresetsMenuByDevicePath(const DevicePath &devicePath)
{
    FSCamera *camera = d->camerasStorage->findCameraByDevicePath(devicePath);

    if (!camera)
        return;

    std::vector<QString> vectorPresetNames = d->camerasStorage->getCameraUserPresetNames(devicePath);

    FSCamerasAndMenuUMap::const_iterator presetMenuIterator = d->umapCamerasAndPresetMenu.find(camera);

    QMenu *presetMenu = nullptr;
    if (presetMenuIterator == d->umapCamerasAndPresetMenu.end()) {
        if (vectorPresetNames.empty()) {
            return;
        }

        FSCamerasAndActionsUMap::const_iterator actionIterator = d->umapCamerasAndActions.find(camera);

        if (actionIterator == d->umapCamerasAndActions.end())
            return;

        presetMenu = createPresetMenu(camera, actionIterator->second);
    } else {
        presetMenu = presetMenuIterator->second;
    }

    updatePresetsMenu(camera, presetMenu, vectorPresetNames);
}

void FSSystemTrayIcon::updateLockPropertiesPauseMenu()
{
    bool isDeletePauseMenu = false;
    bool isDeletePauseRestoreAction = false;

    if ( d->lockPropertiesManager &&
         d->lockPropertiesManager->isExistLockedProperties() ) {
        if (!d->lockPropertiesManager->isLockProperiesPause()) {
            isDeletePauseRestoreAction = true;

            if (!d->lockPropertiesPauseBeforeSeparator) {
                d->lockPropertiesPauseBeforeSeparator = d->mainMenu->insertSeparator(d->cameraSettingsMenu->menuAction());
            }

            if (!d->lockPropertiesPauseMenu) {
                d->lockPropertiesPauseMenu = new QMenu(tr("Lock properties pause"), d->mainMenu);
                d->lockPropertiesPauseMenu->addAction(tr("for 5 minutes"),  this, SLOT(lockPropetries5MinPause()));
                d->lockPropertiesPauseMenu->addAction(tr("for 15 minutes"), this, SLOT(lockPropetries15MinPause()));
                d->lockPropertiesPauseMenu->addAction(tr("for 30 minutes"), this, SLOT(lockPropetries30MinPause()));
                d->lockPropertiesPauseMenu->addAction(tr("for 1 hour"),     this, SLOT(lockPropetries1HourPause()));
                d->lockPropertiesPauseMenu->addAction(tr("for 2 hours"),    this, SLOT(lockPropetries2HoursPause()));
                d->lockPropertiesPauseMenu->addAction(tr("for 4 hours"),    this, SLOT(lockPropetries4HoursPause()));
                d->lockPropertiesPauseMenu->addAction(tr("for 12 hours"),   this, SLOT(lockPropetries12HoursPause()));
                d->lockPropertiesPauseMenu->addAction(tr("for 24 hours"),   this, SLOT(lockPropetries24HoursPause()));
                d->mainMenu->insertMenu(d->lockPropertiesPauseBeforeSeparator, d->lockPropertiesPauseMenu);
            }
        } else {
            isDeletePauseMenu = true;

            if (!d->lockPropertiesPauseBeforeSeparator) {
                d->lockPropertiesPauseBeforeSeparator = d->mainMenu->insertSeparator(d->cameraSettingsMenu->menuAction());
            }

            if (!d->lockPropertiesPauseAbortAction) {
                d->lockPropertiesPauseAbortAction = new QAction(tr("Lock properties restore"), d->mainMenu);
                connect(d->lockPropertiesPauseAbortAction, &QAction::triggered,
                        this,                              &FSSystemTrayIcon::lockProperiesPauseRestore);
                d->mainMenu->insertAction(d->lockPropertiesPauseBeforeSeparator, d->lockPropertiesPauseAbortAction);
            }
        }
    } else {
        isDeletePauseMenu = true;
        isDeletePauseRestoreAction = true;
    }

    if (isDeletePauseMenu && d->lockPropertiesPauseMenu) {
        d->mainMenu->removeAction(d->lockPropertiesPauseMenu->menuAction());

        delete d->lockPropertiesPauseMenu;
        d->lockPropertiesPauseMenu = nullptr;
    }

    if (isDeletePauseRestoreAction && d->lockPropertiesPauseAbortAction) {
        d->mainMenu->removeAction(d->lockPropertiesPauseAbortAction);

        delete d->lockPropertiesPauseAbortAction;
        d->lockPropertiesPauseAbortAction = nullptr;
    }

    if (isDeletePauseMenu && isDeletePauseRestoreAction && d->lockPropertiesPauseBeforeSeparator) {
        d->mainMenu->removeAction(d->lockPropertiesPauseBeforeSeparator);

        delete d->lockPropertiesPauseBeforeSeparator;
        d->lockPropertiesPauseBeforeSeparator = nullptr;
    }
}

void FSSystemTrayIcon::lockPropetries5MinPause()
{
    if (d->lockPropertiesManager)
        d->lockPropertiesManager->lockProperiesPause(5 * 60 * 1000);
}

void FSSystemTrayIcon::lockPropetries15MinPause()
{
    if (d->lockPropertiesManager)
        d->lockPropertiesManager->lockProperiesPause(15 * 60 * 1000);
}

void FSSystemTrayIcon::lockPropetries30MinPause()
{
    if (d->lockPropertiesManager)
        d->lockPropertiesManager->lockProperiesPause(30 * 60 * 1000);
}

void FSSystemTrayIcon::lockPropetries1HourPause()
{
    if (d->lockPropertiesManager)
        d->lockPropertiesManager->lockProperiesPause(60 * 60 * 1000);
}

void FSSystemTrayIcon::lockPropetries2HoursPause()
{
    if (d->lockPropertiesManager)
        d->lockPropertiesManager->lockProperiesPause(2 * 60 * 60 * 1000);
}

void FSSystemTrayIcon::lockPropetries4HoursPause()
{
    if (d->lockPropertiesManager)
        d->lockPropertiesManager->lockProperiesPause(4 * 60 * 60 * 1000);
}

void FSSystemTrayIcon::lockPropetries12HoursPause()
{
    if (d->lockPropertiesManager)
        d->lockPropertiesManager->lockProperiesPause(12 * 60 * 60 * 1000);
}

void FSSystemTrayIcon::lockPropetries24HoursPause()
{
    if (d->lockPropertiesManager)
        d->lockPropertiesManager->lockProperiesPause(24 * 60 * 60 * 1000);
}

void FSSystemTrayIcon::lockProperiesPauseRestore()
{
    if (d->lockPropertiesManager) {
        d->lockPropertiesManager->lockProperiesPauseFinish();
    }
}

void FSSystemTrayIcon::showCameraSettingsDialog()
{
    QObject *object = sender();

    if (!object)
        return;

    QAction *action = qobject_cast<QAction *>(object);

    if (!action)
        return;

    FSCamera *camera = nullptr;

    // Find camera pointer
    {
        FSActionsAndCamerasUMap::const_iterator cameraActionIterator = d->umapActionsAndCameras.find(action);
        if (cameraActionIterator != d->umapActionsAndCameras.end()) {
            camera = cameraActionIterator->second;
        } else {
            QWidget *menuWidget = action->parentWidget();

            if (menuWidget) {
                QMenu *menu = qobject_cast<QMenu *>(menuWidget);

                if (menu) {
                    FSMenuAndCamerasUMap::const_iterator cameraPresetMenuIterator = d->umapPresetMenuAndCameras.find(menu);
                    if (cameraPresetMenuIterator != d->umapPresetMenuAndCameras.end()) {
                        camera = cameraPresetMenuIterator->second;
                    }
                }
            }
        }
    }

    showCameraSettingsDialog(camera);
}

void FSSystemTrayIcon::showCameraSettingsDialog(const DevicePath &devicePath)
{
    if (!d->camerasStorage)
        return;

    showCameraSettingsDialog(d->camerasStorage->findCameraByDevicePath(devicePath));
}

void FSSystemTrayIcon::removeOpenedCameraSettingsDialog(QObject *object)
{
    FSCameraSettingsDialog *dialog = static_cast<FSCameraSettingsDialog *>(object);

    FSOpenedCameraSettingsDialogAndCameraUMap::iterator iterator = d->umapOpenedCameraSettingsDialogAndCamera.find(dialog);

    if (iterator == d->umapOpenedCameraSettingsDialogAndCamera.end())
        return;

    FSCamera *camera = iterator->second;

    d->umapCameraAndOpenedCameraSettingsDialog.erase(camera);
    d->umapOpenedCameraSettingsDialogAndCamera.erase(iterator);
}

void FSSystemTrayIcon::removeUserSettingsDialog(QObject *object)
{
    if (d->cameraSettingsDialog && object == d->cameraSettingsDialog)
        d->cameraSettingsDialog = nullptr;
}

void FSSystemTrayIcon::updateDisplayName(const DevicePath &devicePath)
{
    FSCamera *camera = d->camerasStorage->findCameraByDevicePath(devicePath);

    if (!camera)
        return;

    const auto &actionIterator = d->umapCamerasAndActions.find(camera);
    if (actionIterator != d->umapCamerasAndActions.end()) {
        QAction *action = actionIterator->second;

        if (!action)
            return;

        action->setText(camera->displayName());

        d->cameraSettingsMenu->removeAction(action);
        d->cameraSettingsMenu->insertAction(beforeActionForListActions(d->cameraSettingsMenu->actions(),
                                                                       action->text()),
                                            action);
    }

    const auto &dialogIterator = d->umapCameraAndOpenedCameraSettingsDialog.find(camera);
    if (dialogIterator != d->umapCameraAndOpenedCameraSettingsDialog.end()) {
        FSCameraSettingsDialog *dialog = dialogIterator->second;

        if (!dialog)
            return;

        dialog->updateWindowTitle();
    }
}

void FSSystemTrayIcon::switchCameraPreset()
{
    if (!sender() || !d->camerasStorage)
        return;

    QAction *presetAction = qobject_cast<QAction *>(sender());

    if (!presetAction)
        return;

    FSCamera *camera = nullptr;

    // Find camera pointer
    {
        QWidget *presetMenuWidget = presetAction->parentWidget();

        if (presetMenuWidget) {
            QMenu *presetMenu = qobject_cast<QMenu *>(presetMenuWidget);

            if (presetMenu) {
                FSMenuAndCamerasUMap::const_iterator cameraPresetMenuIterator = d->umapPresetMenuAndCameras.find(presetMenu);
                if (cameraPresetMenuIterator != d->umapPresetMenuAndCameras.end()) {
                    camera = cameraPresetMenuIterator->second;
                }
            }
        }
    }

    if (camera) {
        const QString &presetName = presetAction->text();

        if (!presetName.isEmpty()) {
            FSLockPropertiesManager *lockPropertiesManager = d->lockPropertiesManager;
            if (lockPropertiesManager) {
                if (presetName == TR_MANUAL_TEXT) {
                    lockPropertiesManager->presetUnlockProperies(camera->devicePath());
                    return;
                } else {
                    lockPropertiesManager->presetLockProperties(camera->devicePath(), presetName);
                    return;
                }
            } else {
                FSCameraPropertyValuesUMap umapPropertyValues = d->camerasStorage->getCameraUserPreset(camera->devicePath(), presetName);
                for (const auto &[property, valueParams] : umapPropertyValues) {
                    if (!camera->set(property, valueParams)) {
                        qCritical("%s: Failed to set preset(\"%s\") values(value:\"%ld\", flags:\"%ld\") for property \"%s\", cameraDevicePath=\"%s\"!",
                                  metaObject()->className(),
                                  presetName.toLocal8Bit().constData(),
                                  valueParams.value(),
                                  valueParams.flags(),
                                  fsGetEnumName(property).toLocal8Bit().constData(),
                                  camera->devicePath().toLocal8Bit().constData());
                    } else {
                        return;
                    }
                }
            }
        }
    }

    // Abort checked preset action
    presetAction->setChecked(false);
}
