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

#include "fschangeusercamerasettingdialog.h"
#include "ui_fschangeusercamerasettingdialog.h"

#include <WebCamFS/Camera>
#include <WebCamFS/TranslationsHelper>

class FSChangeUserCameraSettingDialogPrivate
{
public:
    FSChangeUserCameraSettingDialogPrivate();

    FSCamera *camera;

    QString origWindowTitle;
};

FSChangeUserCameraSettingDialogPrivate::FSChangeUserCameraSettingDialogPrivate()
    : camera(nullptr)
{
    // do nothing
}

FSChangeUserCameraSettingDialog::FSChangeUserCameraSettingDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FSChangeUserCameraSettingDialog)
{
    init();
}

FSChangeUserCameraSettingDialog::~FSChangeUserCameraSettingDialog()
{
    delete ui;

    delete d;
    d = nullptr;
}

void FSChangeUserCameraSettingDialog::setCamera(FSCamera *camera,
                                                bool isUpdateAll)
{
    if (d->camera != camera) {
        d->camera = camera;

        if (isUpdateAll) {
            if (d->camera) {
                setUserName(d->camera->userName());
                setBlacklisted(d->camera->isBlackListed());
            } else {
                setUserName(QString());
                setBlacklisted(false);
            }
        }

        updateWindowTitle();
    }
}

FSCamera *FSChangeUserCameraSettingDialog::camera() const
{
    return d->camera;
}

void FSChangeUserCameraSettingDialog::setUserName(const QString &userName)
{
    ui->lineEdit->setText(userName);
}

QString FSChangeUserCameraSettingDialog::userName() const
{
    return ui->lineEdit->text();
}

void FSChangeUserCameraSettingDialog::setBlacklisted(bool isBlacklisted)
{
    ui->checkBox->setChecked(isBlacklisted);
}

bool FSChangeUserCameraSettingDialog::isBlacklisted() const
{
    return ui->checkBox->isChecked();
}

void FSChangeUserCameraSettingDialog::init()
{
    d = new FSChangeUserCameraSettingDialogPrivate();

    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    d->origWindowTitle = windowTitle();

    connect(fsTH, &FSTranslationsHelper::currentLanguageChanged,
            this, &FSChangeUserCameraSettingDialog::retranslate);
}

void FSChangeUserCameraSettingDialog::updateWindowTitle()
{
    if (d->camera && !d->camera->name().isEmpty())
        setWindowTitle(QStringLiteral("%1 \"%2\"").arg(d->origWindowTitle, d->camera->name()));
    else
        setWindowTitle(d->origWindowTitle);
}

void FSChangeUserCameraSettingDialog::retranslate()
{
    ui->retranslateUi(this);

    d->origWindowTitle = windowTitle();
    updateWindowTitle();
}
