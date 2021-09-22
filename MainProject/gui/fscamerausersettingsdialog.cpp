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

#include "fscamerausersettingsdialog.h"
#include "ui_fscamerausersettingsdialog.h"

#include <QLineEdit>
#include <QCheckBox>
#include <QSortFilterProxyModel>

#include <WebCamFS/ItemDelegate>
#include <WebCamFS/CamerasStorage>
#include <WebCamFS/TranslationsHelper>
#include <WebCamFS/CameraPresetsModel>
#include <WebCamFS/CameraSettingsDialog>
#include <WebCamFS/LockPropertiesManager>
#include <WebCamFS/CameraUserSettingsModel>
#include <WebCamFS/CameraLockPropertiesModel>
#include <WebCamFS/CameraDefaultSettingsModel>
#include <WebCamFS/ChangeUserCameraSettingDialog>

class FSCameraUserSettingsDialogPrivate
{
public:
    FSCameraUserSettingsDialogPrivate();

    FSCamerasStorage *camerasStorage;

    FSCameraUserSettingsModel    *cameraUserSettingsModel;
    FSCameraDefaultSettingsModel *cameraDefaultSettingsModel;
    FSCameraPresetsModel         *cameraPresetsModel;
    FSCameraLockPropertiesModel  *cameraLockPropertiesModel;

    QSortFilterProxyModel *cameraUserSettingsProxyModel;
    QSortFilterProxyModel *cameraDefaultSettingsProxyModel;
    QSortFilterProxyModel *cameraPresetsProxyModel;
    QSortFilterProxyModel *cameraLockPropertiesProxyModel;
};

FSCameraUserSettingsDialogPrivate::FSCameraUserSettingsDialogPrivate()
    : camerasStorage(nullptr)
    , cameraUserSettingsModel(nullptr)
    , cameraDefaultSettingsModel(nullptr)
    , cameraPresetsModel(nullptr)
    , cameraLockPropertiesModel(nullptr)
    , cameraUserSettingsProxyModel(nullptr)
    , cameraDefaultSettingsProxyModel(nullptr)
    , cameraPresetsProxyModel(nullptr)
    , cameraLockPropertiesProxyModel(nullptr)
{

}

FSCameraUserSettingsDialog::FSCameraUserSettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FSCameraUserSettingsDialog)
{
    init();
}

FSCameraUserSettingsDialog::~FSCameraUserSettingsDialog()
{
    delete ui;

    delete d->cameraUserSettingsProxyModel;
    delete d->cameraDefaultSettingsProxyModel;
    delete d->cameraPresetsProxyModel;
    delete d->cameraLockPropertiesProxyModel;

    delete d->cameraUserSettingsModel;
    delete d->cameraDefaultSettingsModel;
    delete d->cameraPresetsModel;
    delete d->cameraLockPropertiesModel;

    delete d;
    d = nullptr;
}

void FSCameraUserSettingsDialog::setCamerasStorage(FSCamerasStorage *camerasStorage)
{
    d->camerasStorage = camerasStorage;

    d->cameraUserSettingsModel->setCamerasStorage(camerasStorage);
    d->cameraDefaultSettingsModel->setCamerasStorage(camerasStorage);
    d->cameraPresetsModel->setCamerasStorage(camerasStorage);
    d->cameraLockPropertiesModel->setCamerasStorage(camerasStorage);
}

FSCamerasStorage *FSCameraUserSettingsDialog::camerasStorage() const
{
    return d->camerasStorage;
}

void FSCameraUserSettingsDialog::setLockPropertiesManager(FSLockPropertiesManager *lockPropertiesManager)
{
    d->cameraLockPropertiesModel->setLockPropertiesManager(lockPropertiesManager);
    ui->tabWidget->setTabEnabled(ui->tabWidget->indexOf(ui->tabLockProperties), lockPropertiesManager != nullptr);
}

FSLockPropertiesManager *FSCameraUserSettingsDialog::lockPropertiesManager() const
{
    return d->cameraLockPropertiesModel->lockPropertiesManager();
}

void FSCameraUserSettingsDialog::init()
{
    d = new FSCameraUserSettingsDialogPrivate();

    ui->setupUi(this);
    ui->tabWidget->setCurrentWidget(ui->tabCameraUserSettings);

    connect(fsTH, &FSTranslationsHelper::currentLanguageChanged,
            this, &FSCameraUserSettingsDialog::retranslate);

    connect(ui->tabWidget, &QTabWidget::currentChanged,
            this,          &FSCameraUserSettingsDialog::updateTabData);

    connect(ui->pushButtonChangeCameraUserSettings, SIGNAL(clicked()),
            this,                                     SLOT(on_pushButtonChangeCameraUserSettings()));
    connect(ui->pushButtonChangeDefaultSettings, SIGNAL(clicked()),
            this,                                  SLOT(on_pushButtonChangeDefaultSettings()));
    connect(ui->pushButtonChangePresets, SIGNAL(clicked()),
            this,                          SLOT(on_pushButtonChangePresets()));
    connect(ui->pushButtonChangeLockProperties, SIGNAL(clicked()),
            this,                                 SLOT(on_pushButtonChangeLockProperties()));

    connect(ui->pushButtonClearCameraUserSettings, SIGNAL(clicked()),
            this,                                    SLOT(on_pushButtonClearCameraUserSettings()));
    connect(ui->pushButtonClearDefaultSettings, SIGNAL(clicked()),
            this,                                 SLOT(on_pushButtonClearDefaultSettings()));
    connect(ui->pushButtonClearPresets, SIGNAL(clicked()),
            this,                         SLOT(on_pushButtonClearPresets()));
    connect(ui->pushButtonClearLockProperties, SIGNAL(clicked()),
            this,                                SLOT(on_pushButtonClearLockProperties()));

    FSItemDelegate *itemDelegate = new FSItemDelegate(this);
    ui->treeViewCameraUserSettings->setItemDelegate(itemDelegate);
    ui->treeViewDefaultSettings->setItemDelegate(itemDelegate);
    ui->treeViewPresets->setItemDelegate(itemDelegate);
    ui->treeViewLockProperties->setItemDelegate(itemDelegate);

    d->cameraUserSettingsModel = new FSCameraUserSettingsModel(this);
    d->cameraUserSettingsProxyModel = new QSortFilterProxyModel(d->cameraUserSettingsModel);
    d->cameraUserSettingsProxyModel->setSourceModel(d->cameraUserSettingsModel);
    ui->treeViewCameraUserSettings->setModel(d->cameraUserSettingsProxyModel);
    ui->treeViewCameraUserSettings->setSortingEnabled(true);
    ui->treeViewCameraUserSettings->setTreePosition(d->cameraUserSettingsModel->columnCount()); // Skip tree offset
    ui->treeViewCameraUserSettings->header()->setSectionResizeMode(0, QHeaderView::Interactive);
    ui->treeViewCameraUserSettings->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->treeViewCameraUserSettings->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->treeViewCameraUserSettings->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->treeViewCameraUserSettings->header()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    ui->treeViewCameraUserSettings->sortByColumn(2, Qt::DescendingOrder);

    connect(ui->treeViewCameraUserSettings->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this,                                               SLOT(updateButtonsChangeCameraUserSettings()));

    d->cameraDefaultSettingsModel = new FSCameraDefaultSettingsModel(this);
    d->cameraDefaultSettingsProxyModel = new QSortFilterProxyModel(d->cameraDefaultSettingsModel);
    d->cameraDefaultSettingsProxyModel->setSourceModel(d->cameraDefaultSettingsModel);
    ui->treeViewDefaultSettings->setModel(d->cameraDefaultSettingsProxyModel);
    ui->treeViewDefaultSettings->setSortingEnabled(true);
    ui->treeViewDefaultSettings->setTreePosition(d->cameraDefaultSettingsModel->columnCount()); // Skip tree offset
    ui->treeViewDefaultSettings->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->treeViewDefaultSettings->sortByColumn(1, Qt::DescendingOrder);

    connect(ui->treeViewDefaultSettings->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this,                                            SLOT(updateButtonsChangeDefaultSettings()));

    d->cameraPresetsModel = new FSCameraPresetsModel(this);
    d->cameraPresetsProxyModel = new QSortFilterProxyModel(d->cameraPresetsModel);
    d->cameraPresetsProxyModel->setSourceModel(d->cameraPresetsModel);
    ui->treeViewPresets->setModel(d->cameraPresetsProxyModel);
    ui->treeViewPresets->setSortingEnabled(true);
    ui->treeViewPresets->setTreePosition(d->cameraPresetsModel->columnCount()); // Skip tree offset
    ui->treeViewPresets->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->treeViewPresets->sortByColumn(1, Qt::DescendingOrder);

    connect(ui->treeViewPresets->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this,                                    SLOT(updateButtonsChangePresets()));

    d->cameraLockPropertiesModel = new FSCameraLockPropertiesModel(this);
    d->cameraLockPropertiesProxyModel = new QSortFilterProxyModel(d->cameraLockPropertiesModel);
    d->cameraLockPropertiesProxyModel->setSourceModel(d->cameraLockPropertiesModel);
    ui->treeViewLockProperties->setModel(d->cameraLockPropertiesProxyModel);
    ui->treeViewLockProperties->setSortingEnabled(true);
    ui->treeViewLockProperties->setTreePosition(d->cameraLockPropertiesModel->columnCount()); // Skip tree offset
    ui->treeViewLockProperties->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->treeViewLockProperties->sortByColumn(1, Qt::DescendingOrder);

    connect(ui->treeViewLockProperties->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this,                                           SLOT(updateButtonsLockProperties()));

    connect(ui->treeViewCameraUserSettings, SIGNAL(doubleClicked(QModelIndex)),
            this,                             SLOT(execCameraUserSettingsDialog(QModelIndex)));
    connect(ui->treeViewDefaultSettings, SIGNAL(doubleClicked(QModelIndex)),
            this,                          SLOT(execDefaultSettingsDialog(QModelIndex)));
    connect(ui->treeViewPresets, SIGNAL(doubleClicked(QModelIndex)),
            this,                  SLOT(execPresetSettingsDialog(QModelIndex)));
    connect(ui->treeViewLockProperties, SIGNAL(doubleClicked(QModelIndex)),
            this,                         SLOT(execLockPropertiesDialog(QModelIndex)));

    updateButtonsChangeCameraUserSettings();
    updateButtonsChangeDefaultSettings();
    updateButtonsChangePresets();
    updateButtonsLockProperties();

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->tabWidget->setTabEnabled(ui->tabWidget->indexOf(ui->tabLockProperties), false);
}

void FSCameraUserSettingsDialog::retranslate()
{
    ui->retranslateUi(this);
}

void FSCameraUserSettingsDialog::updateTabData(int index)
{
    QWidget *widget = ui->tabWidget->widget(index);

    if (!widget)
        return;

    if (widget == ui->tabCameraUserSettings) {
        d->cameraUserSettingsModel->updateDevices();
    } else if (widget == ui->tabDefaultSettings) {
        d->cameraDefaultSettingsModel->updateDevices();
    } else if (widget == ui->tabPresets) {
        d->cameraPresetsModel->updateDevices();
    } else if (widget == ui->tabLockProperties) {
        d->cameraLockPropertiesModel->updateDevices();
    }
}

void FSCameraUserSettingsDialog::updateButtonsChangeCameraUserSettings()
{
    if (d->camerasStorage) {
        QModelIndex currentIndex = ui->treeViewCameraUserSettings->currentIndex();

        if (currentIndex.isValid()) {
            currentIndex = d->cameraUserSettingsProxyModel->mapToSource(currentIndex);
        }

        if (currentIndex.isValid()) {
            const QString currentDevicePath = d->cameraUserSettingsModel->devicePath(currentIndex);

            ui->pushButtonChangeCameraUserSettings->setEnabled(currentIndex.isValid());
            ui->pushButtonClearCameraUserSettings->setEnabled(d->camerasStorage->isCameraUserNameUsed(currentDevicePath));
            return;
        }
    }

    ui->pushButtonChangeCameraUserSettings->setEnabled(false);
    ui->pushButtonClearCameraUserSettings->setEnabled(false);
}

void FSCameraUserSettingsDialog::updateButtonsChangeDefaultSettings()
{
    if (d->camerasStorage) {
        QModelIndex currentIndex = ui->treeViewDefaultSettings->currentIndex();

        if (currentIndex.isValid()) {
            currentIndex = d->cameraDefaultSettingsProxyModel->mapToSource(currentIndex);
        }

        if (currentIndex.isValid()) {
            const QString currentDevicePath = d->cameraDefaultSettingsModel->devicePath(currentIndex);

            ui->pushButtonChangeDefaultSettings->setEnabled(currentIndex.isValid());
            ui->pushButtonClearDefaultSettings->setEnabled(d->camerasStorage->isUserDefaultValuesUsed(currentDevicePath));
            return;
        }
    }

    ui->pushButtonChangeDefaultSettings->setEnabled(false);
    ui->pushButtonClearDefaultSettings->setEnabled(false);
}

void FSCameraUserSettingsDialog::updateButtonsChangePresets()
{
    if (d->camerasStorage) {
        QModelIndex currentIndex = ui->treeViewPresets->currentIndex();

        if (currentIndex.isValid()) {
            currentIndex = d->cameraPresetsProxyModel->mapToSource(currentIndex);
        }

        if (currentIndex.isValid()) {
            const QString currentDevicePath = d->cameraPresetsModel->devicePath(currentIndex);

            ui->pushButtonChangePresets->setEnabled(ui->treeViewPresets->currentIndex().isValid());
            ui->pushButtonClearPresets->setEnabled(d->camerasStorage->isCameraUserPresetsUsed(currentDevicePath));
            return;
        }
    }

    ui->pushButtonChangePresets->setEnabled(false);
    ui->pushButtonClearPresets->setEnabled(false);
}

void FSCameraUserSettingsDialog::updateButtonsLockProperties()
{
    if (d->camerasStorage) {
        QModelIndex currentIndex = ui->treeViewLockProperties->currentIndex();

        if (currentIndex.isValid()) {
            currentIndex = d->cameraLockPropertiesProxyModel->mapToSource(currentIndex);
        }

        if (currentIndex.isValid()) {
            const QString currentDevicePath = d->cameraLockPropertiesModel->devicePath(currentIndex);

            ui->pushButtonChangeLockProperties->setEnabled(ui->treeViewLockProperties->currentIndex().isValid());
            ui->pushButtonClearLockProperties->setEnabled(d->camerasStorage->isCameraUserPresetsUsed(currentDevicePath));
            return;
        }
    }

    ui->pushButtonChangeLockProperties->setEnabled(false);
    ui->pushButtonClearLockProperties->setEnabled(false);
}

void FSCameraUserSettingsDialog::on_pushButtonChangeCameraUserSettings()
{
    execCameraUserSettingsDialog(ui->treeViewCameraUserSettings->currentIndex());
}

void FSCameraUserSettingsDialog::on_pushButtonChangeDefaultSettings()
{
    execDefaultSettingsDialog(ui->treeViewDefaultSettings->currentIndex());
}

void FSCameraUserSettingsDialog::on_pushButtonChangePresets()
{
    execPresetSettingsDialog(ui->treeViewPresets->currentIndex());
}

void FSCameraUserSettingsDialog::on_pushButtonChangeLockProperties()
{
    execLockPropertiesDialog(ui->treeViewLockProperties->currentIndex());
}

void FSCameraUserSettingsDialog::on_pushButtonClearCameraUserSettings()
{
    if (!d->camerasStorage)
        return;

    QModelIndex currentIndex = ui->treeViewCameraUserSettings->currentIndex();

    if (currentIndex.isValid()) {
        currentIndex = d->cameraUserSettingsProxyModel->mapToSource(currentIndex);
    }

    if (!currentIndex.isValid())
        return;

    const QString currentDevicePath = d->cameraUserSettingsModel->devicePath(currentIndex);

    if (!currentDevicePath.isEmpty()) {
        d->camerasStorage->cameraUserNameRemove(currentDevicePath);
        d->cameraUserSettingsModel->emitDataByRow(currentDevicePath);
    }
}

void FSCameraUserSettingsDialog::on_pushButtonClearDefaultSettings()
{
    if (!d->camerasStorage)
        return;

    QModelIndex currentIndex = ui->treeViewDefaultSettings->currentIndex();

    if (currentIndex.isValid()) {
        currentIndex = d->cameraDefaultSettingsProxyModel->mapToSource(currentIndex);
    }

    if (!currentIndex.isValid())
        return;

    const QString currentDevicePath = d->cameraDefaultSettingsModel->devicePath(currentIndex);

    if (!currentDevicePath.isEmpty()) {
        d->camerasStorage->userDefaultValuesRemove(currentDevicePath);
        d->cameraDefaultSettingsModel->emitDataByRow(currentDevicePath);
    }
}

void FSCameraUserSettingsDialog::on_pushButtonClearPresets()
{
    if (!d->camerasStorage)
        return;

    QModelIndex currentIndex = ui->treeViewPresets->currentIndex();

    if (currentIndex.isValid()) {
        currentIndex = d->cameraPresetsProxyModel->mapToSource(currentIndex);
    }

    if (!currentIndex.isValid())
        return;

    const QString currentDevicePath = d->cameraPresetsModel->devicePath(currentIndex);

    if (!currentDevicePath.isEmpty()) {
        d->camerasStorage->cameraUserPresetsRemove(currentDevicePath);
        d->cameraPresetsModel->emitDataByRow(currentDevicePath);
    }
}

void FSCameraUserSettingsDialog::on_pushButtonClearLockProperties()
{
    FSLockPropertiesManager *lockPropertiesManager = d->cameraLockPropertiesModel->lockPropertiesManager();

    if (!lockPropertiesManager)
        return;

    QModelIndex currentIndex = ui->treeViewLockProperties->currentIndex();

    if (currentIndex.isValid()) {
        currentIndex = d->cameraLockPropertiesProxyModel->mapToSource(currentIndex);
    }

    if (!currentIndex.isValid())
        return;

    const QString currentDevicePath = d->cameraLockPropertiesModel->devicePath(currentIndex);

    if (!currentDevicePath.isEmpty()) {
        if (lockPropertiesManager->isContaintsPresetLock(currentDevicePath))
            lockPropertiesManager->presetUnlockProperies(currentDevicePath);

        if (lockPropertiesManager->isContaintsManualLockProperties(currentDevicePath))
            lockPropertiesManager->manualUnlockProperties(currentDevicePath);

        d->cameraPresetsModel->emitDataByRow(currentDevicePath);
    }
}

void FSCameraUserSettingsDialog::execCameraUserSettingsDialog(const QModelIndex &index)
{
    if (!d->camerasStorage || !index.isValid() || index.column() == 3 || index.column() == 4)
        return;

    const QModelIndex sourceIndex = d->cameraUserSettingsProxyModel->mapToSource(index);

    if (!sourceIndex.isValid())
        return;

    const DevicePath devicePath = d->cameraUserSettingsModel->devicePath(sourceIndex);

    if (devicePath.isEmpty())
        return;

    FSCamera *camera = d->camerasStorage->findCameraByDevicePath(devicePath);

    FSChangeUserCameraSettingDialog dialog;
    dialog.setCamera(camera);

    if (!camera) {
        dialog.setUserName(d->camerasStorage->getCameraUserName(devicePath));
        dialog.setBlacklisted(d->camerasStorage->isContaintsBlackList(devicePath));
    }

dialog_exec:
    int res = dialog.exec();

    if (res == QDialog::Accepted) {
        if (!d->camerasStorage->setCameraUserName(devicePath, dialog.userName())) {
            goto dialog_exec;
        }

        if (dialog.isBlacklisted())
            d->camerasStorage->insertBlackList(devicePath);
        else
            d->camerasStorage->removeBlackList(devicePath);

        updateButtonsChangeCameraUserSettings();
    }
}

void FSCameraUserSettingsDialog::execDefaultSettingsDialog(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    const QModelIndex sourceIndex = d->cameraDefaultSettingsProxyModel->mapToSource(index);

    if (!sourceIndex.isValid())
        return;

    FSCamera *camera = d->cameraDefaultSettingsModel->camera(sourceIndex);

    if (!camera)
        return;

    FSCameraSettingsDialog dialog(camera, this);
    dialog.setMode(FSCameraSettingsDialog::ChangeDefaultValuesMode);

    int result = dialog.exec();

    if (result == QDialog::Accepted) {
        dialog.sendValuesToCameraStorage();
        updateButtonsChangeDefaultSettings();
    }
}

void FSCameraUserSettingsDialog::execPresetSettingsDialog(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    const QModelIndex sourceIndex = d->cameraPresetsProxyModel->mapToSource(index);

    if (!sourceIndex.isValid())
        return;

    FSCamera *camera = d->cameraPresetsModel->camera(sourceIndex);

    if (!camera)
        return;

    FSCameraSettingsDialog dialog(camera, this);
    dialog.setMode(FSCameraSettingsDialog::ChangePresetMode);

    int result = dialog.exec();

    if (result == QDialog::Accepted) {
        dialog.sendValuesToCameraStorage();
        updateButtonsChangePresets();
    }
}

void FSCameraUserSettingsDialog::execLockPropertiesDialog(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    const QModelIndex sourceIndex = d->cameraLockPropertiesProxyModel->mapToSource(index);

    if (!sourceIndex.isValid())
        return;

    const DevicePath devicePath = d->cameraLockPropertiesModel->devicePath(sourceIndex);

    if (devicePath.isEmpty())
        return;

    emit showCameraSettingsDialog(devicePath);
}
