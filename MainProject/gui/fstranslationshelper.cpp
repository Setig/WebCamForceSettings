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

#include "fstranslationshelper.h"

#include <QDir>
#include <QTimer>
#include <QLocale>
#include <QTranslator>
#include <QCoreApplication>

static FSTranslationsHelper *statTranslationsHelper = nullptr;

class FSTranslationsHelperPrivate
{
public:
    FSTranslationsHelperPrivate();

    QList<QLocale> availableFSLocales;

    // Current loaded translations
    QList<QTranslator *> qtInstalledTranslators;
    QTranslator *fsInstalledTranslator;

    bool isSendEmitCurrentLanguageChanged;

    static QFileInfoList qtTranslationFileInfoList();
    static QFileInfoList fsTranslationFileInfoList();
};

FSTranslationsHelperPrivate::FSTranslationsHelperPrivate()
    : fsInstalledTranslator(nullptr)
    , isSendEmitCurrentLanguageChanged(false)
{
    // do nothing
}

QFileInfoList FSTranslationsHelperPrivate::qtTranslationFileInfoList()
{
    return QDir(qApp->applicationDirPath() + QStringLiteral("/translations/qt"))
            .entryInfoList(QStringList() << QStringLiteral("qt_*.qm") << QStringLiteral("qtbase_*.qm"),
                           QDir::Files);
}

QFileInfoList FSTranslationsHelperPrivate::fsTranslationFileInfoList()
{
    return QDir(qApp->applicationDirPath() + QStringLiteral("/translations"))
            .entryInfoList(QStringList() << QStringLiteral("wcfs_*.qm"),
                           QDir::Files);
}

FSTranslationsHelper *FSTranslationsHelper::instance()
{
    if (!statTranslationsHelper) {
        statTranslationsHelper = new FSTranslationsHelper();
    }

    return statTranslationsHelper;
}

void FSTranslationsHelper::uninitialization()
{
    delete statTranslationsHelper;
    statTranslationsHelper = nullptr;
}

QList<QLocale> FSTranslationsHelper::availableFSLocales() const
{
    return d->availableFSLocales;
}

void FSTranslationsHelper::updateAvailableFSLocales()
{
    QList<QLocale> availableFSLocales;

    const QVector<QTranslator *> translators = const_cast<FSTranslationsHelper *>(this)->loadFSTranslations();
    for (QTranslator *translator : translators) {
        availableFSLocales.push_back(getLocaleFromTranslatorFileName(translator));
        delete translator;
    }

    d->availableFSLocales = availableFSLocales;
}

bool FSTranslationsHelper::setCurrentLocale(const QLocale &locale)
{
    if ( !d->fsInstalledTranslator ||
         ( d->fsInstalledTranslator &&
           getLocaleFromTranslatorFileName(d->fsInstalledTranslator) != locale ) ) {
        resetCurrentTranslations();

        bool isOk = installFSTranslation(locale);

        if (isOk)
            installQtTranslations(locale);

        return isOk;
    }

    return true;
}

QLocale FSTranslationsHelper::currentLocale() const
{
    if (d->fsInstalledTranslator)
        return QLocale(getLocaleFromTranslatorFileName(d->fsInstalledTranslator));

    return QLocale::system();
}

FSTranslationsHelper::FSTranslationsHelper(QObject *parent)
    : QObject(parent)
{
    init();
}

FSTranslationsHelper::~FSTranslationsHelper()
{
    resetCurrentTranslations();

    delete d;
    d = nullptr;
}

void FSTranslationsHelper::init()
{
    d = new FSTranslationsHelperPrivate();

    updateAvailableFSLocales();
}

void FSTranslationsHelper::resetCurrentTranslations()
{
    resetCurrentFSTranslation();
    resetCurrentQtTranslations();
}

void FSTranslationsHelper::resetCurrentFSTranslation()
{
    if (d->fsInstalledTranslator) {
        if (qApp->removeTranslator(d->fsInstalledTranslator)) {
            delete d->fsInstalledTranslator;
            d->fsInstalledTranslator = nullptr;
            sendEmitCurrentLanguageChanged();
        } else {
            qCritical("%s: Failed to remove translation, file path=\"%s\"!",
                      staticMetaObject.className(),
                      d->fsInstalledTranslator->filePath().toLocal8Bit().constData());
            d->fsInstalledTranslator = nullptr;
        }
    }
}

void FSTranslationsHelper::resetCurrentQtTranslations()
{
    for (QTranslator *translator : qAsConst(d->qtInstalledTranslators)) {
        if (qApp->removeTranslator(translator)) {
            delete translator;
            sendEmitCurrentLanguageChanged();
        } else {
            qCritical("%s: Failed to remove translation, file path=\"%s\"!",
                      staticMetaObject.className(),
                      translator->filePath().toLocal8Bit().constData());
        }
    }

    d->qtInstalledTranslators.clear();
}

bool FSTranslationsHelper::installFSTranslation(const QLocale &locale)
{
    const QFileInfoList translationFileInfoList = d->fsTranslationFileInfoList();
    for (const QFileInfo &fileInfo : translationFileInfoList) {
        if (compareLocale(locale, getLocaleFromTranslatorFileName(fileInfo.filePath()))) {
            QTranslator *translator = new QTranslator(this);

            if (!translator->load(fileInfo.baseName(), fileInfo.dir().absolutePath())) {
                delete translator;
                continue;
            }

            if (qApp->installTranslator(translator)) {
                d->fsInstalledTranslator = translator;
                sendEmitCurrentLanguageChanged();
                return true;
            }

            qCritical("%s: Failed to install translation, file path=\"%s\"!",
                      staticMetaObject.className(),
                      translator->filePath().toLocal8Bit().constData());
            delete translator;
            break;
        }
    }

    return false;
}

void FSTranslationsHelper::installQtTranslations(const QLocale &locale)
{
    const QFileInfoList translationFileInfoList = d->qtTranslationFileInfoList();
    for (const QFileInfo &fileInfo : translationFileInfoList) {
        if (compareLocale(locale, getLocaleFromTranslatorFileName(fileInfo.filePath()))) {
            QTranslator *translator = new QTranslator(this);

            if ( translator->load(fileInfo.baseName(), fileInfo.dir().absolutePath()) &&
                 qApp->installTranslator(translator) ) {
                d->qtInstalledTranslators.push_back(translator);
                sendEmitCurrentLanguageChanged();
                continue;
            }

            delete translator;
            continue;
        }
    }
}

QVector<QTranslator *> FSTranslationsHelper::loadFSTranslations()
{
    QList<QTranslator *> result;

    const QFileInfoList translationFileInfoList = d->fsTranslationFileInfoList();
    for (const QFileInfo &fileInfo : translationFileInfoList) {
        QTranslator *translator = new QTranslator(this);

        if (!translator->load(fileInfo.baseName(), fileInfo.dir().absolutePath())) {
            delete translator;
            continue;
        }

        result.push_back(translator);
    }

    return result.toVector();
}

bool FSTranslationsHelper::compareLocale(const QLocale &locale1,
                                         const QLocale &locale2)
{
    return (locale1.language() == locale2.language() &&
            locale1.country() == locale2.country());
}

QLocale FSTranslationsHelper::getLocaleFromTranslatorFileName(QTranslator *translator)
{
    if (translator) {
        return getLocaleFromTranslatorFileName(translator->filePath());
    }

    return QLocale();
}

QLocale FSTranslationsHelper::getLocaleFromTranslatorFileName(const QString &filePath)
{
    const QString baseName = QFileInfo(filePath).baseName();
    QString localeName;

    // TODO: Remake for different locale formats
    if ( baseName.count() > 6 &&
         baseName[baseName.count() - 3] == '_' &&
         baseName[baseName.count() - 6] == '_' ) {
        localeName = baseName.right(5);
    } else if ( baseName.count() > 3 &&
                baseName[baseName.count() - 3] == '_' ) {
        localeName = baseName.right(2);
    }

    return QLocale(localeName);
}

void FSTranslationsHelper::sendEmitCurrentLanguageChanged()
{
    if (!d->isSendEmitCurrentLanguageChanged) {
        d->isSendEmitCurrentLanguageChanged = true;
        QTimer::singleShot(0, this, &FSTranslationsHelper::emitCurrentLanguageChanged);
    }
}

void FSTranslationsHelper::emitCurrentLanguageChanged()
{
    d->isSendEmitCurrentLanguageChanged = false;
    emit currentLanguageChanged();
}
