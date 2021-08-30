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

namespace Ui { class FSSettingsDialog; }

class QCheckBox;
class QLabel;
class QSpinBox;

class FSSettingsDialogPrivate;

class FSSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FSSettingsDialog(QWidget *parent = nullptr);
    ~FSSettingsDialog() override;

    void setCurrentLocale(const QLocale &locale);
    QLocale currentLocale() const;

    void updateLocales();

    void setAutoStartEnable(bool isEnable);
    bool isAutoStartEnable() const;

    void setAutoStart(bool isAutoStart);
    bool isAutoStart() const;

    void setCustomStyleIndex(int styleIndex);
    int customStyleIndex() const;

    void setCameraDetectionEnable(bool isEnable);
    bool isCameraDetectionEnable() const;

    void setCameraDetectionInterval(int msec);
    int cameraDetectionInterval() const;

    void setCameraValueUpdateEnable(bool isEnable);
    bool isCameraValueUpdateEnable() const;

    void setCameraValueUpdateInterval(int msec);
    int cameraValueUpdateInterval() const;

    void setLockPropertiesEnable(bool isEnable);
    bool isLockPropertiesEnable() const;

    void setLockPropertiesInterval(int msec);
    int lockPropertiesInterval() const;

private:
    FSSettingsDialogPrivate *d;
    Ui::FSSettingsDialog *ui;

    void init();

    void connectionCheckBoxWithLabelAndSpinBox(QCheckBox *checkBox,
                                               QLabel *label,
                                               QSpinBox *spinBox);

    static QString genLocaleName(const QLocale &locale);

    static QStringList getCustomStyleNames();
    static QString getCustomStyleName(int styleIndex);
    static int getCustomStyleIndex(const QString &styleName);

private slots:
    void retranslate();

    void restoreDefaultValues();

    void updateCurrentLanguage();
};
