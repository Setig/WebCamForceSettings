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

#include <QCoreApplication>

#include <QFileInfo>

#include <WebCamFS/Lib>
#include <WebCamFS/Camera>
#include <WebCamFS/AppArguments>

void printHelp()
{
    fprintf(stdout,
            "Usage: %s [OPTION]\n"
            "The console application is designed to work with webcam info.\n\n"
            "OPTIONS:\n"
            "%s, %s\tprint info for all available web cameras(video input devices) in the OS\n"
            "%s, %s\t\tdisplay this help and exit\n"
            "    %s\tdisplay help for gui application and exit\n"
            "    %s\toutput version information and exit\n",
            FS_CONSOLE_NAME,
            SHOW_CAMERAS_INFO_SHORT_ARG_NAME,
            SHOW_CAMERAS_INFO_LONG_ARG_NAME,
            HELP_SHORT_ARG_NAME,
            HELP_LONG_ARG_NAME,
            HELP_GUI_ARG_NAME,
            VERSION_LONG_ARG_NAME);
    fflush(stdout);
}

void printGUIHelp()
{
    fprintf(stdout,
            "Usage: %s [OPTION]\n"
            "The application is designed to work with webcam settings.\n\n"
            "OPTIONS:\n"
            "%s, %s=\"devicePath\"\n\tshow dialog web camera settings by device path\n\t(devicePath for you webcam can be found using cameras info)\n\n"
            "%s\tcreate log file in directory:\n"
            "\t\"%s\"\n",
            FS_GUI_NAME,
            SHOW_CAMERA_SETTINGS_DIALOG_SHORT_ARG_NAME,
            SHOW_CAMERA_SETTINGS_DIALOG_LONG_ARG_NAME,
            CREATE_LOG_FILE_SHORT_ARG_NAME,
            "C:\\Users\\%USERNAME%\\AppData\\Local\\Temp\\WebCamFS\\logs");
    fflush(stdout);
}

void printVersion()
{
    fprintf(stdout,
            "%s (%s)\nLibrary version : %s\nApplication version : %s\nAuthor by %s.\n",
            FS_CONSOLE_NAME,
            qApp->applicationName().toLocal8Bit().constData(),
            fsLibraryVersion().toLocal8Bit().constData(),
            qApp->applicationVersion().toLocal8Bit().constData(),
            "Alexander Tsyganov");
    fflush(stdout);
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    a.setApplicationName(FS_CONSOLE_APPLICATION_NAME);
    a.setApplicationVersion(FS_PROJECT_GLOBAL_VERSION);

    // Application arguments parsing
    {
        bool isShowCamerasInfo = false;

        const QStringList appArgs = a.arguments().mid(1);

        for (int i = 0; i < appArgs.count(); i++) {
            const QString &arg = appArgs.at(i);

            if ( arg == HELP_SHORT_ARG_NAME ||
                 arg == HELP_LONG_ARG_NAME ) {
                printHelp();
                return 1;
            } else if (arg == HELP_GUI_ARG_NAME) {
                printGUIHelp();
                return 1;
            } else if (arg == VERSION_LONG_ARG_NAME) {
                printVersion();
                return 1;
            } else if ( arg == SHOW_CAMERAS_INFO_SHORT_ARG_NAME ||
                        arg == SHOW_CAMERAS_INFO_LONG_ARG_NAME ) {
                isShowCamerasInfo = true;
                break;
            } else {
                fprintf(stderr, "Unknown application argument \"%s\"!\n", arg.toLocal8Bit().constData());
                fflush(stderr);
            }
        }

        if (!FSCamera::initializeWinCOMLibrary()) {
            qFatal("Cannot initialize the windows COM library!");
            return 1;
        }

        if (isShowCamerasInfo) {
            FSCamera::printCamerasInfo();
            FSCamera::uninitializeWinCOMLibrary();
            return 0;
        }
    }

    printHelp();
    return 1;
}
