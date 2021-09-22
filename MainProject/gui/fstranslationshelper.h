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

#include <QObject>

class QTranslator;

#define fsTH FSTranslationsHelper::instance()

class FSTranslationsHelperPrivate;

class FSTranslationsHelper : public QObject
{
    Q_OBJECT

public:
    static FSTranslationsHelper *instance();
    static void uninitialization();

    QList<QLocale> availableFSLocales() const;

    void updateAvailableFSLocales();

    bool setCurrentLocale(const QLocale &locale);
    QLocale currentLocale() const;

signals:
    void currentLanguageChanged();

private:
    explicit FSTranslationsHelper(QObject *parent = nullptr);
    ~FSTranslationsHelper() override;

    FSTranslationsHelperPrivate *d;

    void init();

    void resetCurrentTranslations();

    void resetCurrentFSTranslation();
    void resetCurrentQtTranslations();

    bool installFSTranslation(const QLocale &locale);
    void installQtTranslations(const QLocale &locale);

    QVector<QTranslator *> loadFSTranslations();

    static bool compareLocale(const QLocale &locale1, const QLocale &locale2);

    static QLocale getLocaleFromTranslatorFileName(QTranslator *translator);
    static QLocale getLocaleFromTranslatorFileName(const QString &filePath);

private slots:
    void sendEmitCurrentLanguageChanged();
    void emitCurrentLanguageChanged();
};
