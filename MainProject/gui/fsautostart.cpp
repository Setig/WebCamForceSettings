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

#include "fsautostart.h"

#include <tchar.h>
#include <stdio.h>
#include <windows.h>

#include <vector>

#include <QApplication>

#define RUN_REG_NAME  "Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#define APP_REG_VALUE "WebCam Force Settings"

#define START_BUFFER_SIZE MAX_PATH

void FSAutoStart::setAutoStart(bool isAutoStart, FSAutoStartLastError *lastError)
{
    HKEY newValue;
    LONG fres = RegOpenKey(HKEY_CURRENT_USER, TEXT(RUN_REG_NAME), &newValue);
    if (fres != ERROR_SUCCESS) {
        qCritical("Unable to open registry key; last error = %ld!", fres);
        if (lastError) { *lastError = FSAutoStartLastError(1001, fres); }
        return;
    }

    if (isAutoStart) {
        std::vector<TCHAR> vecModuleFilePath(START_BUFFER_SIZE);

        DWORD pathLen = GetModuleFileName(NULL, &vecModuleFilePath.at(0), DWORD(vecModuleFilePath.size()));
        DWORD wLastError = GetLastError();
        int i = 0;

        while (wLastError == ERROR_INSUFFICIENT_BUFFER) {
            if (i > 100) {
                break;
            }
            i++;

            vecModuleFilePath.resize(vecModuleFilePath.size() * 1.5);

            pathLen = GetModuleFileName(NULL, &vecModuleFilePath.at(0), DWORD(vecModuleFilePath.size()));
            wLastError = GetLastError();
        }

        if (pathLen == 0) {
            qCritical("Unable to get module file name; last error = %ld!", wLastError);
            if (lastError) { *lastError = FSAutoStartLastError(1002, wLastError); }
            return;
        }

        fres = RegSetValueEx(newValue, TEXT(APP_REG_VALUE), 0, REG_SZ, (LPBYTE)&vecModuleFilePath.at(0), pathLen * sizeof(TCHAR));

        if (fres != ERROR_SUCCESS) {
            RegCloseKey(newValue);
            qCritical("Unable to set registry value; last error = %ld!", fres);
            if (lastError) { *lastError = FSAutoStartLastError(1003, fres); }
            return;
        }
    } else {
        fres = RegDeleteValue(newValue, TEXT(APP_REG_VALUE));
        if (fres != ERROR_SUCCESS) {
            RegCloseKey(newValue);

            if (fres == ERROR_FILE_NOT_FOUND) {
                if (lastError) { *lastError = FSAutoStartLastError(); }
                return;
            }

            qCritical("Unable to delete registry value; last error = %ld!", fres);
            if (lastError) { *lastError = FSAutoStartLastError(1004, fres); }
            return;
        }
    }

    RegCloseKey(newValue);

    if (lastError) { *lastError = FSAutoStartLastError(); }
}

bool FSAutoStart::isAutoStart(FSAutoStartLastError *lastError)
{
    HKEY newValue;
    LONG fres = RegOpenKey(HKEY_CURRENT_USER, TEXT(RUN_REG_NAME), &newValue);
    if (fres != ERROR_SUCCESS) {
        qCritical("Unable to open registry key; last error = %ld!", fres);
        if (lastError) { *lastError = FSAutoStartLastError(1, fres); }
        return false;
    }

    DWORD lpType = REG_NONE;
    std::vector<TCHAR> vecModuleFilePathFromReg(START_BUFFER_SIZE);
    DWORD pathLenFromReg = DWORD(vecModuleFilePathFromReg.size());

    fres = RegQueryValueEx(newValue, TEXT(APP_REG_VALUE), 0, &lpType, (LPBYTE)&vecModuleFilePathFromReg.at(0), &pathLenFromReg);
    int i = 0;

    if (fres == ERROR_MORE_DATA && pathLenFromReg > vecModuleFilePathFromReg.size()) {
        vecModuleFilePathFromReg.resize(pathLenFromReg);
        fres = RegQueryValueEx(newValue, TEXT(APP_REG_VALUE), 0, &lpType, (LPBYTE)&vecModuleFilePathFromReg.at(0), &pathLenFromReg);
    }

    while(fres == ERROR_MORE_DATA) {
        if (i > 100) {
            break;
        }
        i++;

        vecModuleFilePathFromReg.resize(vecModuleFilePathFromReg.size() * 1.5);
        pathLenFromReg = DWORD(vecModuleFilePathFromReg.size());

        fres = RegQueryValueEx(newValue, TEXT(APP_REG_VALUE), 0, &lpType, (LPBYTE)&vecModuleFilePathFromReg.at(0), &pathLenFromReg);
    }

    if (fres != ERROR_SUCCESS) {
        RegCloseKey(newValue);

        if (fres == ERROR_FILE_NOT_FOUND) {
            if (lastError) { *lastError = FSAutoStartLastError(); }
            return false;
        }

        qCritical("Unable to query registry value; last error = %ld!", fres);
        if (lastError) { *lastError = FSAutoStartLastError(2, fres); }
        return false;
    }

    RegCloseKey(newValue);

    std::vector<TCHAR> vecModuleFilePath(START_BUFFER_SIZE);

    DWORD pathLen = GetModuleFileName(NULL, &vecModuleFilePath.at(0), DWORD(vecModuleFilePath.size()));
    DWORD wLastError = GetLastError();
    i = 0;

    while (wLastError == ERROR_INSUFFICIENT_BUFFER) {
        if (i > 100) {
            break;
        }
        i++;

        vecModuleFilePath.resize(vecModuleFilePath.size() * 1.5);

        pathLen = GetModuleFileName(NULL, &vecModuleFilePath.at(0), DWORD(vecModuleFilePath.size()));
        wLastError = GetLastError();
    }

    if (pathLen == 0) {
        qCritical("Unable to get module file name; last error = %ld!", wLastError);
        if (lastError) { *lastError = FSAutoStartLastError(3, wLastError); }
        return false;
    }

    if ( lpType == REG_SZ && _tcscmp(&vecModuleFilePath.at(0), &vecModuleFilePathFromReg.at(0)) == 0) {
        if (lastError) { *lastError = FSAutoStartLastError(); }
        return true;
    }

    if (lastError) { *lastError = FSAutoStartLastError(); }
    return false;
}

FSAutoStart::FSAutoStart()
{

}

FSAutoStart::~FSAutoStart()
{

}
