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

#include "fsrenamepresetnamedialog.h"
#include "ui_fsrenamepresetnamedialog.h"

#include <WebCamFS/TranslationsHelper>

FSRenamePresetNameDialog::FSRenamePresetNameDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FSRenamePresetNameDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    connect(fsTH, &FSTranslationsHelper::currentLanguageChanged,
            this, &FSRenamePresetNameDialog::retranslate);
}

FSRenamePresetNameDialog::~FSRenamePresetNameDialog()
{
    delete ui;
}

void FSRenamePresetNameDialog::setPresetName(const QString &presetName)
{
    ui->lineEdit->setText(presetName);
}

QString FSRenamePresetNameDialog::presetName() const
{
    return ui->lineEdit->text();
}

void FSRenamePresetNameDialog::retranslate()
{
    ui->retranslateUi(this);
}
