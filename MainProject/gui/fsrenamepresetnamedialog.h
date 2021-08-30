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

namespace Ui { class FSRenamePresetNameDialog; }

class FSRenamePresetNameDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FSRenamePresetNameDialog(QWidget *parent = nullptr);
    ~FSRenamePresetNameDialog();

    void setPresetName(const QString &presetName);
    QString presetName() const;

private:
    Ui::FSRenamePresetNameDialog *ui;

private slots:
    void retranslate();
};
