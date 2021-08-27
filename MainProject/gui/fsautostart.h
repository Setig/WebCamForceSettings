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

struct FSAutoStartLastError
{
public:
    FSAutoStartLastError()
        : m_step(-1)
        , m_error(0)
    {
        // do nothing
    }

    FSAutoStartLastError(int step, long error)
        : m_step(step)
        , m_error(error)
    {
        // do nothing
    }

    inline bool isSuccess() const
    {
        return (m_step == -1) && (m_error == 0);
    }

    inline int step() const
    {
        return m_step;
    }

    inline unsigned long error() const
    {
        return m_error;
    }

private:
    int m_step;
    unsigned long m_error;
};

class FSAutoStart
{
public:
    static void setAutoStart(bool isAutoStart, FSAutoStartLastError *lastError = nullptr);
    static bool isAutoStart(FSAutoStartLastError *lastError = nullptr);

private:
    FSAutoStart();
    ~FSAutoStart();
};
