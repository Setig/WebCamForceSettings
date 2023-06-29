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

#include <QApplication>

#include <QDir>
#include <QDateTime>
#include <QStandardPaths>

#ifdef BUILD_WITH_QT_SINGLE_APPLICATION
#include <QMessageBox>
#include <qtsingleapplication.h>
#endif // BUILD_WITH_QT_SINGLE_APPLICATION

#ifdef BUILD_WITH_Q_EASY_SETTINGS
#include "qeasysettings.hpp"
#endif // BUILD_WITH_Q_EASY_SETTINGS

#include <WebCamFS/Camera>
#include <WebCamFS/Settings>
#include <WebCamFS/IconCreator>
#include <WebCamFS/AppArguments>
#include <WebCamFS/SystemTrayIcon>
#include <WebCamFS/CamerasStorage>
#include <WebCamFS/TranslationsHelper>
#include <WebCamFS/CameraSettingsDialog>

#ifdef SHOW_ONLY_PROJECT_ICON
#include <QLabel>
#endif // SHOW_ONLY_PROJECT_ICON

int showCameraSettingsDialogByDevicePath(const DevicePath &devicePath,
                                         bool isUseDefaultSettings)
{
    FSCamera *camera = FSCamera::findCameraByDevicePath(devicePath);

    if (!camera) {
        qCritical("Could't find webcam with devicePath(\"%s\")!\n", devicePath.toLocal8Bit().constData());
        delete camera;
        return 1;
    }

    FSSettings::initialization();
    fsTH->setCurrentLocale(FSSettings::currentLocale());

    FSCamerasStorage *camerasStorage = nullptr;

    if (isUseDefaultSettings) {
        camerasStorage = new FSCamerasStorage();
        camerasStorage->loadDefaultValueParams();
        camera->setCamerasStorage(camerasStorage);
    }

#ifdef BUILD_WITH_Q_EASY_SETTINGS
    QEasySettings::setStyle(QEasySettings::Style(isUseDefaultSettings ?
                                                     FSSettings::customStyleIndex() :
                                                     FSSettings::defaultCustomStyleIndex()));
#endif // BUILD_WITH_Q_EASY_SETTINGS

    FSCameraSettingsDialog dialog(camera);

    dialog.exec();

    delete camera;

    if (camerasStorage)
        delete camerasStorage;

    return 0;
}

FILE *fsStandartErrorStream = stdout;

void fsMessageOutput(QtMsgType type,
                     const QMessageLogContext &context,
                     const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    QByteArray localCurrentTime = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz").toLocal8Bit();
    QByteArray localMsgTypeLabel;

    switch (type) {
    case QtDebugMsg:    localMsgTypeLabel = QByteArrayLiteral("D"); break;
    case QtInfoMsg:     localMsgTypeLabel = QByteArrayLiteral("I"); break;
    case QtWarningMsg:  localMsgTypeLabel = QByteArrayLiteral("W"); break;
    case QtCriticalMsg: localMsgTypeLabel = QByteArrayLiteral("C"); break;
    case QtFatalMsg:    localMsgTypeLabel = QByteArrayLiteral("F"); break;
    }

    if (context.file || context.function)
        fprintf(fsStandartErrorStream, "[%s][%s]: \"%s\" (%s:%u, %s)\n", localCurrentTime.constData(), localMsgTypeLabel.constData(), localMsg.constData(), context.file, context.line, context.function);
    else
        fprintf(fsStandartErrorStream, "[%s][%s]: \"%s\"\n", localCurrentTime.constData(), localMsgTypeLabel.constData(), localMsg.constData());

    fflush(fsStandartErrorStream);
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(fsMessageOutput);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
#endif

    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::RoundPreferFloor);

#ifdef BUILD_WITH_QT_SINGLE_APPLICATION
    QtSingleApplication a(argc, argv);
#else
    QApplication a(argc, argv);
#endif // BUILD_WITH_QT_SINGLE_APPLICATION

    a.setWindowIcon(QIcon(FSIconCreator::generate()));
    a.setApplicationName(QStringLiteral(FS_GUI_APPLICATION_NAME));
    a.setApplicationVersion(QStringLiteral(FS_PROJECT_GLOBAL_VERSION));

#ifdef SHOW_ONLY_PROJECT_ICON
    int size = 128;
    QPixmap pixmap = FSIconCreator::generate(size);
    pixmap.save(qApp->applicationDirPath() + QStringLiteral(".png"), QStringLiteral("PNG"));

    QLabel label;
    label.setScaledContents(true);
    label.setPixmap(pixmap);
    label.setMinimumSize(size, size);
    label.setWindowFlags(Qt::SplashScreen);
    label.show();
    return a.exec();
#endif // SHOW_ONLY_PROJECT_ICON

    FILE *logFile = NULL;

    if (!FSCamera::initializeWinCOMLibrary()) {
        qFatal("Cannot initialize the windows COM library!");
        return 1;
    }

    // Application arguments parsing
    {
        int showCameraSettingsDialogArgIndex = -1;
        bool isUseDefaultSettings = false;

        const QStringList appArgs = a.arguments().mid(1);

        for (int i = 0; i < appArgs.count(); i++) {
            const QString &arg = appArgs.at(i);

            if (arg == QLatin1String(SHOW_CAMERA_SETTINGS_DIALOG_SHORT_ARG_NAME)) {
                showCameraSettingsDialogArgIndex = i;
                i++; // Not read next argument
                continue;
            } else if (arg.startsWith(QLatin1String(SHOW_CAMERA_SETTINGS_DIALOG_LONG_ARG_NAME "="))) {
                showCameraSettingsDialogArgIndex = i;
                continue;
            } else if ( arg == QLatin1String(USE_USER_DEFAULT_SETTINGS_SHORT_ARG_NAME) ||
                        arg == QLatin1String(USE_USER_DEFAULT_SETTINGS_LONG_ARG_NAME) ) {
                isUseDefaultSettings = true;
            } else if (arg == QLatin1String(CREATE_LOG_FILE_SHORT_ARG_NAME)) {
                const QString dirPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QStringLiteral("/WebCamFS/logs");
                QDir().mkpath(dirPath);
                QByteArray logFileName = QStringLiteral("%1/%2.log").arg(dirPath,
                                                                  QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh-mm-ss"))).toLocal8Bit();
                logFile = fopen(logFileName.constData(), "a");

                if (logFile != NULL) {
                    qInfo("Create log file \"%s\". All qt message redirecting to log file!", logFileName.constData());
                    fsStandartErrorStream = logFile;
                } else {
                    qCritical("Cannot create log file \"%s\"!", logFileName.constData());
                }
            } else {
                qCritical("Unknown application argument \"%s\"!\n", arg.toLocal8Bit().constData());
            }
        }

        if (showCameraSettingsDialogArgIndex != -1) {
            DevicePath devicePath;

            const QString &arg = appArgs.at(showCameraSettingsDialogArgIndex);

            if (arg == QLatin1String(SHOW_CAMERA_SETTINGS_DIALOG_SHORT_ARG_NAME)) {
                if (showCameraSettingsDialogArgIndex != appArgs.count() - 1) {
                    devicePath = appArgs.at(showCameraSettingsDialogArgIndex + 1);
                } else {
                    qCritical("Not set device path!\n");
                    return 1;
                }
            } else if (arg.startsWith(QLatin1String(SHOW_CAMERA_SETTINGS_DIALOG_LONG_ARG_NAME "="))) {
                devicePath = arg.right(arg.count() - QStringLiteral(SHOW_CAMERA_SETTINGS_DIALOG_LONG_ARG_NAME "=").count());
            }

            if (devicePath.startsWith('"') && devicePath.endsWith('"')) {
                devicePath = devicePath.mid(1, devicePath.count() - 2);
            }

            if (!devicePath.isEmpty()) {
                return showCameraSettingsDialogByDevicePath(devicePath, isUseDefaultSettings);
            } else {
                qCritical("Device path is empty!\n");
                return 1;
            }
        }
    }

    FSSettings::initialization();
    fsTH->setCurrentLocale(FSSettings::currentLocale());

#ifdef BUILD_WITH_QT_SINGLE_APPLICATION
    if (a.isRunning()) {
        QMessageBox::information(nullptr,
                                 FSSystemTrayIcon::tr("Information"),
                                 FSSystemTrayIcon::tr("The application is already running."));
        return 1;
    }
#endif // SHOW_ONLY_PROJECT_ICON

    a.setQuitOnLastWindowClosed(false);

    FSSystemTrayIcon trayIcon;
    trayIcon.show();

    const int code = a.exec();

    if (logFile) {
        fclose(logFile);
    }

    return code;
}
