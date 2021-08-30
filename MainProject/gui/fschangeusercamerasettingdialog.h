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

#include <QDialog>

class FSCamera;

namespace Ui { class FSChangeUserCameraSettingDialog; }

class FSChangeUserCameraSettingDialogPrivate;

class FSChangeUserCameraSettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FSChangeUserCameraSettingDialog(QWidget *parent = nullptr);
    ~FSChangeUserCameraSettingDialog();

    void setCamera(FSCamera *camera,
                   bool isUpdateAll = true);
    FSCamera *camera() const;

    void setUserName(const QString &userName);
    QString userName() const;

    void setBlacklisted(bool isBlacklisted);
    bool isBlacklisted() const;

private:
    FSChangeUserCameraSettingDialogPrivate *d;
    Ui::FSChangeUserCameraSettingDialog *ui;

    void init();

    void updateWindowTitle();

private slots:
    void retranslate();
};
