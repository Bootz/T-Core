/*
 * Copyright (C) 2005 - 2011 MaNGOS <http://www.getmangos.org/>
 *
 * Copyright (C) 2008 - 2011 TrinityCore <http://www.trinitycore.org/>
 *
 * Copyright (C) 2011 TrilliumEMU <http://www.trilliumemu.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Log.h"
#include "LogMgr.h"
#include "Config.h"

Log::Log()
{
    _DebugLogMask = DebugLogFilters(sConfig->GetIntDefault("Log.Debug.Mask", LOG_FILTER_NONE));
}

Log::~Log() { }

void Log::outString(const char* fmt, ...)
{
    va_list lst;
    va_start(lst, fmt);
    sLogMgr->WriteLn(SERVER_LOG, LOGL_STRING, fmt, lst);
    va_end(lst);
}

void Log::outString()
{
    sLogMgr->WriteLn(SERVER_LOG, LOGL_STRING, std::string());
}

void Log::outStringInLine(const char* fmt, ...)
{
    va_list lst;
    va_start(lst, fmt);
    sLogMgr->Write(SERVER_LOG, LOGL_STRING, false, fmt, lst);
    va_end(lst);
}

void Log::outCrash(const char* fmt, ...)
{
    va_list lst;
    va_start(lst, fmt);
    sLogMgr->WriteLn(SERVER_LOG, LOGL_CRASH, fmt, lst);
    va_end(lst);
}

void Log::outError(const char* fmt, ...)
{
    va_list lst;
    va_start(lst, fmt);
    sLogMgr->WriteLn(SERVER_LOG, LOGL_ERROR, fmt, lst);
    va_end(lst);
}

void Log::outSQLDev(const char* fmt, ...)
{
    va_list lst;
    va_start(lst, fmt);
    sLogMgr->WriteLn(SQLDEVELOPER_LOG, LOGL_STRING, fmt, lst);
    va_end(lst);
}

void Log::outErrorDb(const char* fmt, ...)
{
    va_list lst;
    va_start(lst, fmt);
    sLogMgr->WriteLn(DBERROR_LOG, LOGL_ERROR, fmt, lst);
    va_end(lst);
}

void Log::outBasic(const char* fmt, ...)
{
    va_list lst;
    va_start(lst, fmt);
    sLogMgr->WriteLn(SERVER_LOG, LOGL_WARNING, fmt, lst);
    va_end(lst);
}

void Log::outDetail(const char* fmt, ...)
{
    va_list lst;
    va_start(lst, fmt);
    sLogMgr->WriteLn(SERVER_LOG, LOGL_INFO, fmt, lst);
    va_end(lst);
}

void Log::outDebugInLine(const char* fmt, ...)
{
    va_list lst;
    va_start(lst, fmt);
    sLogMgr->Write(SERVER_LOG, LOGL_FULL, false, fmt, lst);
    va_end(lst);
}

void Log::outDebug(DebugLogFilters f, const char* fmt, ...)
{
    if (!(_DebugLogMask & f))
        return;

    va_list lst;
    va_start(lst, fmt);
    sLogMgr->WriteLn(SERVER_LOG, LOGL_FULL, fmt, lst);
    va_end(lst);
}

void Log::outStaticDebug(const char* fmt, ...)
{
    va_list lst;
    va_start(lst, fmt);
    sLogMgr->WriteLn(SERVER_LOG, LOGL_FULL, fmt, lst);
    va_end(lst);
}

bool Log::IsOutDebug() const
{
    return sLogMgr->ShouldLog(SERVER_LOG, LOGL_FULL);
}