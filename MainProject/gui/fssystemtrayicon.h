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

#include <QSystemTrayIcon>

#include <WebCamFS/Structs>

class QAction;

class FSSystemTrayIconPrivate;

class FSSystemTrayIcon : public QSystemTrayIcon
{
    Q_OBJECT

public:
    explicit FSSystemTrayIcon(QObject *parent = nullptr);
    ~FSSystemTrayIcon() override;

private:
    FSSystemTrayIconPrivate *d;

    void init();
    void initMainMenu();

    void registerCamera(const FSCameraData &cameraData);
    void unregisterCamera(FSCamera *camera);

    void createCameraMenuObjects(FSCamera *camera);
    void deleteCameraMenuObjects(FSCamera *camera);

    QAction *beforeActionForListActions(const QList<QAction *> &listActions,
                                        const QString &newActionText);

    void updateCamerasDetectionTimer();
    void updateSettingsDialogs();
    void updateLockPropertiesManager();

    QMenu *createPresetMenu(FSCamera *camera, QAction *cameraAction);

    void updatePresetsMenu(FSCamera *camera,
                           QMenu *presetMenu,
                           const std::vector<QString> &presetNames);

private slots:
    void retranslate();

    void about();
    void execSettingsDialog();
    void execUserSettingsDialog();

    void onShowAvailableCamerasMenu();
    void checkAvailableCameras();

    void updateActionsByBlackList(const DevicePath &devicePath);

    void updatePresetsMenuByCamera(FSCamera *camera);
    void updatePresetsMenuByDevicePath(const DevicePath &devicePath);

    void updateLockPropertiesPauseMenu();

    void lockPropetries5MinPause();
    void lockPropetries15MinPause();
    void lockPropetries30MinPause();
    void lockPropetries1HourPause();
    void lockPropetries2HoursPause();
    void lockPropetries4HoursPause();
    void lockPropetries12HoursPause();
    void lockPropetries24HoursPause();

    void lockProperiesPauseRestore();

    void showCameraSettingsDialog();

    void removeOpenedCameraSettingsDialog(QObject *object);
    void removeSettingsDialog(QObject *object);
    void removeUserSettingsDialog(QObject *object);

    void updateDisplayName(const DevicePath &devicePath);

    void switchCameraPreset();
};
