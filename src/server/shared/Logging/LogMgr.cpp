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

#include "LogMgr.h"
#include "Config.h"
#include "Util.h"
#include "Database/DatabaseEnv.h"

inline void appendPathSeparator(std::string& s)
{
    if (!s.empty())
        if ((s.at(s.length() - 1) != '/') && (s.at(s.length() - 1) != '\\'))
            s.push_back('/');
}

inline int makedir(const char *path, mode_t mode)
{
    int status = 0;
    // Directory does not exist
#if PLATFORM == PLATFORM_WINDOWS
    struct _stat st;
    if (_stat(path, &st) != 0)
    {
        if (mkdir(path) != 0)
#else
    struct stat st;
    if (stat(path, &st) != 0)
    {
        if (mkdir(path, mode) != 0)
#endif
            status = -1;
    }
    else if ((st.st_mode & S_IFMT) != S_IFDIR)
    {
        errno = ENOTDIR;
        status = -1;
    }
    return status;
}

LogMgr::LogFile::LogFile(const std::string& name, PhysicalLogFile* physicalLog, bool enabled, LogLevel level, bool consoleFlag, bool dbFlag)
    : _name(name), _physicalLog(physicalLog), _enabled(enabled), _level(level), _consoleFlag(consoleFlag), _dbFlag(dbFlag)
{
}

LogMgr::LogFile::~LogFile()
{
    Close();
}

void LogMgr::LogFile::Close()
{
    //sLogMgr->UnregisterLogFile(_name.c_str());
}

void LogMgr::LogFile::Write(LogLevel level, bool addNewLine, bool withTime, const std::string& msg)
{
    // Do not write to DB if inline function is used
    if (addNewLine && sLogMgr->IsLogDb(level) && _dbFlag)
        WriteDb(level, msg);
    if (level <= _level)
    {
        if (_consoleFlag)
        {
            if (addNewLine)
                sLogMgr->WriteConsoleLn(level, msg);
            else
                sLogMgr->WriteConsole(level, msg);
        }
        if (_physicalLog)
            _physicalLog->Write(level, addNewLine, withTime, msg);
    }
}

void LogMgr::LogFile::Write(LogLevel level, bool addNewLine, bool withTime, const char* fmt, va_list& lst)
{
    // Do not write to DB if inline function is used
    if (addNewLine && sLogMgr->IsLogDb(level) && _dbFlag)
    {
        va_list args;
        va_copy(args, lst);
        WriteDb(level, fmt, args);
        va_end(args);
    }
    if (level <= _level)
    {
        if (_consoleFlag)
        {
            va_list args;
            va_copy(args, lst);
            if (addNewLine)
                sLogMgr->WriteConsoleLn(level, fmt, args);
            else
                sLogMgr->WriteConsole(level, fmt, args);
            va_end(args);
        }
        if (_physicalLog)
            _physicalLog->Write(level, addNewLine, withTime, fmt, lst);
    }
}

void LogMgr::LogFile::WriteDb(LogLevel level, const std::string& msg)
{
    if (!msg.empty())
    {
        PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_ADD_LOG);
        stmt->setUInt32(0, uint32(time(NULL)));
        stmt->setUInt32(1, sLogMgr->GetRealmId());
        stmt->setString(2, _name);
        stmt->setInt8  (3, int8(level));
        stmt->setString(4, msg);
        LoginDatabase.Execute(stmt);
    }
}

void LogMgr::LogFile::WriteDb(LogLevel level, const char* fmt, va_list& lst)
{
    if (fmt)
    {
        char sz[MAX_QUERY_LEN];
        vsnprintf(sz, MAX_QUERY_LEN, fmt, lst);
        std::string msg(sz);
        WriteDb(level, msg);
    }
}

void LogMgr::LogFile::Flush()
{
    if (_physicalLog)
        _physicalLog->Flush();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Physical log
LogMgr::PhysicalLogFile::PhysicalLogFile(const std::string& dir, const std::string& fileName, const std::string& timeStampFmt, bool dateSplit, bool isAppend, uint32 flushBytes) :
    _dir(dir), _fileName(fileName), _timeStampFmt(timeStampFmt), _dateSplit(dateSplit), _isAppend(isAppend), _flushBytes(flushBytes),
    _file(NULL), _writtenLength(0), _refCount(1), _lastDay(0)
{
    Open();
}

LogMgr::PhysicalLogFile::~PhysicalLogFile()
{
    Close();
}

uint8 LogMgr::PhysicalLogFile::IncreaseRefCount(uint8 amount)
{
    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_lock);
    _refCount += amount;
    return _refCount;
}

uint8 LogMgr::PhysicalLogFile::DecreaseRefCount(uint8 amount)
{
    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_lock);
    if (_refCount >= amount)
        _refCount -= amount;
    else
        _refCount = 0;
    return _refCount;
}

void LogMgr::PhysicalLogFile::Open()
{
    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_lock);
    if (!_fileName.empty() && !_file)
    {
        std::string path(_dir);
        time_t t = time(NULL);
        tm* aTm = localtime(&t);
        if (_dateSplit)
            _lastDay = aTm->tm_mday;
        // Format time
        char szDate[TRILLIUM_PATH_MAX];
        if (strftime(szDate, sizeof(szDate), _fileName.c_str(), aTm) == 0)
            path += _fileName;
        else
            path += szDate;
        // Make sure path exists
        _file = LogMgr::OpenFile(path, _isAppend);
    }
}

void LogMgr::PhysicalLogFile::Close()
{
    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_lock);
    if (_file) {
        fclose(_file);
        _file = NULL;
    }
}

void LogMgr::PhysicalLogFile::UpdateSettings(const std::string& timeStampFmt, bool dateSplit, bool isAppend, uint32 flushBytes)
{
    _timeStampFmt = timeStampFmt;
    // If someone specifies both to split and not to split dates, then prefer split
    _dateSplit |= dateSplit;
    // If someone specifies both to append or to overwrite file, then prefer append
    _isAppend |= isAppend;
    // Prefer minimum number of bytes to write
    _flushBytes = std::min(_flushBytes, flushBytes);
    // Increase reference count
    IncreaseRefCount();
}

void LogMgr::PhysicalLogFile::Write(LogLevel level, bool addNewLine, bool withTime, const char* fmt, va_list& lst)
{
    _CheckDate();

    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_lock);
    if (_file)
    {
        // Time
        _writtenLength += LogMgr::OutTimestamp(_file, _timeStampFmt);
        // Message
        _writtenLength += LogMgr::WritePrefix(_file, level);
        _writtenLength += vfprintf(_file, fmt, lst);
        if (addNewLine)
            _writtenLength += fprintf(_file, "\n");
        // Flush to disk if necessary amount of bytes were written
        if ((_flushBytes != LogFlushManual) && (_writtenLength >= _flushBytes))
            Flush();
    }
}

// I know it is bad to copy/paste, but I'm too lazy now to invent something
void LogMgr::PhysicalLogFile::Write(LogLevel level, bool addNewLine, bool withTime, const std::string& msg)
{
    _CheckDate();

    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_lock);
    if (_file)
    {
        // Time
        _writtenLength += LogMgr::OutTimestamp(_file, _timeStampFmt);
        // Message
        _writtenLength += LogMgr::WritePrefix(_file, level);
        _writtenLength += fprintf(_file, "%s", msg.c_str());
        if (addNewLine)
            _writtenLength += fprintf(_file, "\n");
        // Flush to disk if necessary amount of bytes were written
        if ((_flushBytes != LogFlushManual) && (_writtenLength >= _flushBytes))
            Flush();
    }
}

void LogMgr::PhysicalLogFile::Flush()
{
    fflush(_file);
    _writtenLength = 0;
}

inline void LogMgr::PhysicalLogFile::_CheckDate()
{
    // Check date switch
    if (_dateSplit)
    {
        time_t t = time(NULL);
        tm* aTm = localtime(&t);
        // Reopen file if date changed
        if (_lastDay != aTm->tm_mday)
        {
            Close();
            Open();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Log manager
LogMgr::LogMgr() : _realmId(0), _logDb(false), _logDbLevel(LOGL_WARNING), _logConsoleLevel(LOGL_WARNING),
    _dumpCharacters(false), _dumpCharactersSeparate(false), _useColor(false), _logGmPerAccount(false)
{
    Initialize();
}

LogMgr::~LogMgr()
{
    Clear();
}

void LogMgr::Clear()
{
    // Delete logical logs
    for (LogsMap::iterator itr = _logsMap.begin(); itr != _logsMap.end(); ++itr)
        delete itr->second;
    _logsMap.clear();
    // Delete resources
    for (PhysicalLogs::iterator itr = _physicalLogs.begin(); itr != _physicalLogs.end(); ++itr)
        delete (*itr);
    _physicalLogs.clear();
    // Clear map
    _physicalLogsMap.clear();
}

void LogMgr::Initialize()
{
    Clear();

    _dir = sConfig->GetStringDefault("Log.Directory", "");
    appendPathSeparator(_dir);
    // Colors
    _InitColors(sConfig->GetStringDefault("Log.Console.Colors", ""));
    _logConsoleLevel = LogLevel(sConfig->GetIntDefault("Log.Console.Level", LOGL_WARNING));
    // Chars dump
    _dumpCharacters = sConfig->GetBoolDefault("Log.Char.Dump", false);
    _dumpCharactersSeparate = sConfig->GetBoolDefault("Log.Char.Dump.Separate", false);
    if (_dumpCharactersSeparate)
    {
        _dumpCharactersDir = sConfig->GetStringDefault("Log.Char.Dump.Directory", "");
        appendPathSeparator(_dumpCharactersDir);
    }
    // Initialize all common logs here
    RegisterLogFile(SERVER_LOG);
    RegisterLogFile(DBERROR_LOG);
    RegisterLogFile(RA_LOG);
    RegisterLogFile(CHAR_LOG);
    RegisterLogFile(ARENA_LOG);
    RegisterLogFile(SQLDRIVER_LOG);
    RegisterLogFile(SQLDEVELOPER_LOG);
    RegisterLogFile(SOCKET_LOG);
    // GM
    _logGmPerAccount = sConfig->GetBoolDefault("Log.GM.PerAccount", false);
    if (!_logGmPerAccount)
        RegisterLogFile(GM_LOG);
    else
    {
        std::string setting("Log.");
        setting.append(GM_LOG);
        if (sConfig->GetBoolDefault(std::string(setting + ".Enabled").c_str(), false))
        {
            _gmFilePath = sConfig->GetStringDefault(std::string(setting + ".File").c_str(), "");
            _gmTimestampFmt = sConfig->GetStringDefault(std::string(setting + ".TimestampFmt").c_str(), "%Y-%m-%d %H:%M:%S");
        }
    }
}

void LogMgr::RegisterLogFile(const char* logName)
{
    if (_GetLog(logName))
    {
        WriteConsoleLn(LOGL_ERROR, "LogMgr: log '%s' is already registered!", logName);
        return;
    }
    // Load log settings
    std::string setting("Log.");
    setting.append(logName);

    bool enabled = sConfig->GetBoolDefault(std::string(setting + ".Enabled").c_str(), false);
    std::string fileName = sConfig->GetStringDefault(std::string(setting + ".File").c_str(), "");
    LogLevel level = LogLevel(sConfig->GetIntDefault(std::string(setting + ".Level").c_str(), LOGL_WARNING));
    std::string timeStampFmt = sConfig->GetStringDefault(std::string(setting + ".TimestampFmt").c_str(), "%Y-%m-%d %H:%M:%S");
    bool dateSplit = sConfig->GetBoolDefault(std::string(setting + ".SplitDate").c_str(), true);
    bool isAppend = sConfig->GetBoolDefault(std::string(setting + ".Append").c_str(), true);
    bool consoleFlag = sConfig->GetBoolDefault(std::string(setting + ".Console").c_str(), false);
    bool dbFlag = sConfig->GetBoolDefault(std::string(setting + ".DB").c_str(), false);
    uint32 flushBytes = sConfig->GetIntDefault(std::string(setting + ".FlushBytes").c_str(), 0);
    // Check if there is already PhysicalLog for given file path
    PhysicalLogFile* log = NULL;
    for (PhysicalLogs::iterator itr = _physicalLogs.begin(); itr != _physicalLogs.end(); ++itr)
    {
        if ((*itr)->GetFileName() == fileName)
        {
            // Yes, there is
            log = (*itr);
            log->UpdateSettings(timeStampFmt, dateSplit, isAppend, flushBytes);
            break;
        }
    }
    if (enabled)
    {
        // No, there is not
        if (!log)
        {
            log = new PhysicalLogFile(_dir, fileName, timeStampFmt, dateSplit, isAppend, flushBytes);
            _physicalLogs.push_back(log);
        }
        _physicalLogsMap[logName] = log;
    }
    // Create and save logical log file
    _logsMap[logName] = new LogFile(logName, log, enabled, level, consoleFlag, dbFlag);

    if (enabled && !fileName.empty())
        WriteConsoleLn(LOGL_STRING, "LogMgr: log '%s' succesfully registered for file '%s'", logName, fileName.c_str());
    else
        WriteConsoleLn(LOGL_STRING, "LogMgr: log '%s' is disabled", logName);
}

void LogMgr::UnregisterLogFile(const char* logName)
{
    if (LogFile* log = _GetLog(logName))
    {
        // Delete physical log
        if (PhysicalLogFile* physicalLog = log->GetPhysicalLog())
            if (!physicalLog->DecreaseRefCount())
                delete physicalLog;
        // Remove information from log maps
        _physicalLogsMap.erase(logName);
        _logsMap.erase(logName);
        delete log;

        WriteConsoleLn(LOGL_STRING, "LogMgr: successfully unregistered log '%s'!", logName);
    }
    else
        WriteConsoleLn(LOGL_ERROR, "LogMgr: trying to unregister log '%s' which is not found!", logName);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Write routines
///////////////////////////////////////////////////////////////////////////////////////////////////
// Internal
inline void LogMgr::_Write(const char* logName, LogLevel level, bool appendNewLine, bool withTime, const std::string& msg)
{
    if (LogFile* log = _GetLog(logName))
    {
        if (log->IsEnabled())
            log->Write(level, appendNewLine, withTime, msg);
    }
    else
        WriteConsoleLn(LOGL_ERROR, "LogMgr: trying to write into log '%s' which is not found!", logName);
}

inline void LogMgr::_Write(const char* logName, LogLevel level, bool appendNewLine, bool withTime, const char* fmt, va_list& lst)
{
    if (LogFile* log = _GetLog(logName))
    {
        if (log->IsEnabled())
            log->Write(level, appendNewLine, withTime, fmt, lst);
    }
    else
        WriteConsoleLn(LOGL_ERROR, "LogMgr: trying to write into log '%s' which is not found!", logName);
}

// Write message appending new line
void LogMgr::WriteLn(const char* logName, const std::string& msg)
{
    _Write(logName, LOGL_STRING, true, true, msg);
}

void LogMgr::WriteLn(const char* logName, const char* fmt, va_list& lst)
{
    _Write(logName, LOGL_STRING, true, true, fmt, lst);
}

void LogMgr::WriteLn(const char* logName, const char* fmt, ...)
{
    if (LogFile* log = _GetLog(logName))
    {
        if (log->IsEnabled())
        {
            va_list lst;
            va_start(lst, fmt);
            log->Write(LOGL_STRING, true, true, fmt, lst);
            va_end(lst);
        }
    }
    else
        WriteConsoleLn(LOGL_ERROR, "LogMgr: trying to write into log '%s' which is not found!", logName);
}

void LogMgr::WriteLn(const char* logName, LogLevel level, const std::string& msg)
{
    _Write(logName, level, true, true, msg);
}

void LogMgr::WriteLn(const char* logName, LogLevel level, const char* fmt, va_list& lst)
{
    _Write(logName, level, true, true, fmt, lst);
}

void LogMgr::WriteLn(const char* logName, LogLevel level, const char* fmt, ...)
{
    if (LogFile* log = _GetLog(logName))
    {
        if (log->IsEnabled())
        {
            va_list lst;
            va_start(lst, fmt);
            log->Write(level, true, true, fmt, lst);
            va_end(lst);
        }
    }
    else
        WriteConsoleLn(LOGL_ERROR, "LogMgr: trying to write into log '%s' which is not found!", logName);
}

// Write message as is optionally adding timestamp before message
void LogMgr::Write(const char* logName, bool withTime, const std::string& msg)
{
    _Write(logName, LOGL_STRING, false, withTime, msg);
}

void LogMgr::Write(const char* logName, bool withTime, const char* fmt, va_list& lst)
{
    _Write(logName, LOGL_STRING, false, withTime, fmt, lst);
}

void LogMgr::Write(const char* logName, bool withTime, const char* fmt, ...)
{
    if (LogFile* log = _GetLog(logName))
    {
        if (log->IsEnabled())
        {
            va_list lst;
            va_start(lst, fmt);
            log->Write(LOGL_STRING, false, withTime, fmt, lst);
            va_end(lst);
        }
    }
    else
        WriteConsoleLn(LOGL_ERROR, "LogMgr: trying to write into log '%s' which is not found!", logName);
}

void LogMgr::Write(const char* logName, LogLevel level, bool withTime, const std::string& msg)
{
    _Write(logName, level, false, withTime, msg);
}

void LogMgr::Write(const char* logName, LogLevel level, bool withTime, const char* fmt, va_list& lst)
{
    _Write(logName, level, false, withTime, fmt, lst);
}

void LogMgr::Write(const char* logName, LogLevel level, bool withTime, const char* fmt, ...)
{
    if (LogFile* log = _GetLog(logName))
    {
        if (log->IsEnabled())
        {
            va_list lst;
            va_start(lst, fmt);
            log->Write(level, false, withTime, fmt, lst);
            va_end(lst);
        }
    }
    else
        WriteConsoleLn(LOGL_ERROR, "LogMgr: trying to write into log '%s' which is not found!", logName);
}

void LogMgr::Flush(const char* logName)
{
    if (LogFile* log = _GetLog(logName))
    {
        if (log->IsEnabled())
            log->Flush();
    }
    else
        WriteConsoleLn(LOGL_ERROR, "LogMgr: trying to flush log '%s' which is not found!", logName);
}

// Write message to file and close handle
void LogMgr::WriteFile(const std::string& path, bool isAppend, const std::string& msg)
{
    if (!path.empty())
    {
        if (FILE* file = OpenFile(GetLogDirectory() + path, isAppend))
        {
            fprintf(file, "%s\n", msg.c_str());
            fflush(file);
            fclose(file);
        }
    }
}

void LogMgr::WriteFile(const std::string& path, bool isAppend, const char* fmt, va_list& lst)
{
    if (!path.empty())
    {
        if (FILE* file = OpenFile(GetLogDirectory() + path, isAppend))
        {
            vfprintf(file, fmt, lst);
            fprintf(file, "\n");
            fflush(file);
            fclose(file);
        }
    }
}

void LogMgr::WriteFile(const std::string& path, bool isAppend, const char* fmt, ...)
{
    if (!path.empty())
    {
        if (FILE* file = OpenFile(GetLogDirectory() + path, isAppend))
        {
            va_list lst;
            va_start(lst, fmt);
            vfprintf(file, fmt, lst);
            va_end(lst);
            fprintf(file, "\n");
            fflush(file);
            fclose(file);
        }
    }
}

// Enable/disable log
void LogMgr::SetLogEnabled(const char* logName, bool enabled)
{
    if (LogFile* log = _GetLog(logName))
        log->SetEnabled(enabled);
    else
        WriteConsoleLn(LOGL_ERROR, "LogMgr: trying to changed enabled state of log '%s' which is not found!", logName);
}

bool LogMgr::IsLogEnabled(const char* logName) const
{
    bool res = false;
    if (LogFile const* log = _GetLog(logName))
        res = log->IsEnabled();
    else
        WriteConsoleLn(LOGL_ERROR, "LogMgr: trying to get enabled state of log '%s' which is not found!", logName);
    return res;
}

bool LogMgr::ToggleLogEnabled(const char* logName)
{
    bool res = false;
    if (LogFile* log = _GetLog(logName))
    {
        res = !log->IsEnabled();
        log->SetEnabled(res);
    }
    else
        WriteConsoleLn(LOGL_ERROR, "LogMgr: trying to toggle enabled state of log '%s' which is not found!", logName);
    return res;
}

// Misc
void LogMgr::SetLogDb()
{
    _logDb = sConfig->GetBoolDefault("Log.DB.Enable", false);
    _logDbLevel = LogLevel(sConfig->GetIntDefault("Log.DB.Level", LOGL_WARNING));
}

void LogMgr::WriteCharacterDump(uint32 accountId, uint32 guid, const char* name, const char* data)
{
    if (_dumpCharactersSeparate)
    {
        char sz[29]; // Max length: name(12) + guid(11) + _.log(5) + \0
        snprintf(sz, 29, "%d_%s.log", guid, name);
        std::string fileName(_dumpCharactersDir);
        fileName.append(sz);

        WriteFile(fileName, false, "== START DUMP == (account: %u guid: %u name: %s )\n%s\n== END DUMP ==\n",
                  accountId, guid, name, data);
    }
    else
        WriteLn(CHAR_LOG, "== START DUMP == (account: %u guid: %u name: %s )\n%s\n== END DUMP ==\n",
                accountId, guid, name, data);
}

// Custom write commands
void LogMgr::WriteGmCommand(uint32 accountId, const char* fmt, ...)
{
    if (_logGmPerAccount)
    {
        if (!_gmFilePath.empty())
        {
            std::string path;

            time_t t = time(NULL);
            tm* aTm = localtime(&t);
            // Replace time format specifiers (may not exist)
            char sz[TRILLIUM_PATH_MAX];
            if (strftime(sz, sizeof(sz), _gmFilePath.c_str(), aTm) == 0)
                path = _gmFilePath;
            else
                path = sz;
            // Replace $a with account ID
            std::string::size_type pos = path.find("$a");
            if (pos != std::string::npos)
            {
                char szAccount[12];
                snprintf(szAccount, 12, "%u", accountId);
                path.replace(pos, 2, szAccount);
            }

            LogMgr::OutTimestamp(GetLogDirectory() + path, _gmTimestampFmt);

            va_list lst;
            va_start(lst, fmt);
            WriteFile(path, true, fmt, lst);
            va_end(lst);

            va_start(lst, fmt);
            WriteConsoleLn(LOGL_WARNING, fmt, lst);
            va_end(lst);
        }
    }
    else
    {
        va_list lst;
        va_start(lst, fmt);
        WriteLn(GM_LOG, LOGL_WARNING, fmt, lst);
        va_end(lst);
    }
}

// Log levels
void LogMgr::SetLogLevel(const char* logName, uint32 level)
{
    if (LogFile* log = _GetLog(logName))
    {
        log->SetLogLevel((level >= MAX_LOGLEVEL) ? LOGL_WARNING : LogLevel(level));
        WriteConsoleLn(LOGL_STRING, "Log level of log '%s' is set to %u", logName, log->GetLogLevel());
    }
    else
        WriteConsoleLn(LOGL_ERROR, "LogMgr: trying to set log level of log '%s' which is not found!", logName);
}

void LogMgr::SetConsoleLogLevel(uint32 level)
{
    _logConsoleLevel = (level >= MAX_LOGLEVEL) ? LOGL_WARNING : LogLevel(level);
    WriteConsoleLn(LOGL_STRING, "Console log level is set to %u", _logConsoleLevel);
}

void LogMgr::SetDbLogLevel(uint32 level)
{
    _logDbLevel = (level >= MAX_LOGLEVEL) ? LOGL_WARNING : LogLevel(level);
    WriteConsoleLn(LOGL_STRING, "DB log level is set to %u", _logDbLevel);
}

// Write to console
inline void LogMgr::_WriteConsole(LogLevel level, bool appendNewLine, const std::string& msg) const
{
    if (level <= _logConsoleLevel)
    {
        ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_consoleLock);
        bool isError = level < LOGL_STRING;
        // Change color only for messages with new line (not inline)
        if (_useColor && appendNewLine)
            SetConsoleColor(isError, isError ? LOGC_ERROR : _colors[level]);

        FILE* f = isError ? stderr : stdout;
        WritePrefix(f, level);
        utf8printf(f, msg.c_str());
        if (appendNewLine)
            fprintf(f, "\n");
        fflush(f);

        if (_useColor && appendNewLine)
            ResetConsoleColor(isError);
    }
}

inline void LogMgr::_WriteConsole(LogLevel level, bool appendNewLine, const char* fmt, va_list& lst) const
{
    if (level <= _logConsoleLevel)
    {
        ACE_Guard<ACE_Recursive_Thread_Mutex> guard(_consoleLock);
        bool isError = level < LOGL_STRING;
        // Change color only for messages with new line (not inline)
        if (_useColor && appendNewLine)
            SetConsoleColor(isError, isError ? LOGC_ERROR : _colors[level]);

        FILE* f = isError ? stderr : stdout;
        WritePrefix(f, level);
        vutf8printf(f, fmt, lst);
        if (appendNewLine)
            fprintf(f, "\n");
        fflush(f);

        if (_useColor && appendNewLine)
            ResetConsoleColor(isError);
    }

}

void LogMgr::WriteConsoleLn(LogLevel level, const std::string& msg) const
{
    _WriteConsole(level, true, msg);
}

void LogMgr::WriteConsoleLn(LogLevel level, const char* fmt, va_list& lst) const
{
    _WriteConsole(level, true, fmt, lst);
}

void LogMgr::WriteConsoleLn(LogLevel level, const char* fmt, ...) const
{
    va_list lst;
    va_start(lst, fmt);
    _WriteConsole(level, true, fmt, lst);
    va_end(lst);
}

void LogMgr::WriteConsole(LogLevel level, const std::string& msg) const
{
    _WriteConsole(level, false, msg);
}

void LogMgr::WriteConsole(LogLevel level, const char* fmt, va_list& lst) const
{
    _WriteConsole(level, false, fmt, lst);
}

void LogMgr::WriteConsole(LogLevel level, const char* fmt, ...) const
{
    va_list lst;
    va_start(lst, fmt);
    _WriteConsole(level, false, fmt, lst);
    va_end(lst);
}

// static
uint32 LogMgr::OutTimestamp(const std::string& path, const std::string& timeStampFmt)
{
    uint32 res = 0;
    if (FILE* file = OpenFile(path, true))
    {
        res = OutTimestamp(file, timeStampFmt);
        fflush(file);
        fclose(file);
    }
    return res;
}

// static
uint32 LogMgr::OutTimestamp(FILE* file, const std::string& timeStampFmt)
{
    time_t t = time(NULL);
    tm* aTm = localtime(&t);
    char sz[TRILLIUM_PATH_MAX];
    strftime(sz, sizeof(sz), timeStampFmt.c_str(), aTm);
    return fprintf(file, "%s ", sz);
}

// static
bool LogMgr::CreatePath(const std::string& path)
{
    const mode_t mode = S_IRWXU;
    char* szPath = (char*)path.c_str();
    char* sp;
    int status = 0;
    char* pp = szPath;
    while (status == 0 && (sp = strchr(pp, '/')))
    {
        if (sp != pp)
        {
            *sp = '\0';
            status = makedir(szPath, mode);
            *sp = '/';
        }
        pp = sp + 1;
    }
    return (status == 0);
}

// static
void LogMgr::SetConsoleColor(bool isError, LogColor color)
{
#if PLATFORM == PLATFORM_WINDOWS
    static WORD WinColorFG[MAX_LOGCOLOR] =
    {
        0,                                                          // BLACK
        FOREGROUND_RED,                                             // RED
        FOREGROUND_GREEN,                                           // GREEN
        FOREGROUND_RED | FOREGROUND_GREEN,                          // BROWN
        FOREGROUND_BLUE,                                            // BLUE
        FOREGROUND_RED | FOREGROUND_BLUE,                           // MAGENTA
        FOREGROUND_GREEN | FOREGROUND_BLUE,                         // CYAN
        FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,        // WHITE
        FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,   // YELLOW
        FOREGROUND_RED | FOREGROUND_INTENSITY,                      // RED_BOLD
        FOREGROUND_GREEN | FOREGROUND_INTENSITY,                    // GREEN_BOLD
        FOREGROUND_BLUE | FOREGROUND_INTENSITY,                     // BLUE_BOLD
        FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY,    // MAGENTA_BOLD
        FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,  // CYAN_BOLD
        FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY  // WHITE_BOLD
    };

    HANDLE hConsole = GetStdHandle(isError ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, WinColorFG[color]);
#else
    enum ANSITextAttr
    {
        TA_NORMAL   = 0,
        TA_BOLD     = 1,
        TA_BLINK    = 5,
        TA_REVERSE  = 7
    };

    enum ANSIFgTextAttr
    {
        FG_BLACK = 30,
        FG_RED,
        FG_GREEN,
        FG_BROWN,
        FG_BLUE,
        FG_MAGENTA,
        FG_CYAN,
        FG_WHITE,
        FG_YELLOW
    };

    enum ANSIBgTextAttr
    {
        BG_BLACK = 40,
        BG_RED,
        BG_GREEN,
        BG_BROWN,
        BG_BLUE,
        BG_MAGENTA,
        BG_CYAN,
        BG_WHITE
    };

    static uint8 UnixColorFG[MAX_LOGCOLOR] =
    {
        FG_BLACK,                                           // BLACK
        FG_RED,                                             // RED
        FG_GREEN,                                           // GREEN
        FG_BROWN,                                           // BROWN
        FG_BLUE,                                            // BLUE
        FG_MAGENTA,                                         // MAGENTA
        FG_CYAN,                                            // CYAN
        FG_WHITE,                                           // GRAY
        FG_YELLOW,                                          // YELLOW
        FG_RED,                                             // LIGHTRED
        FG_GREEN,                                           // LIGTHGREEN
        FG_BLUE,                                            // LIGTHBLUE
        FG_MAGENTA,                                         // LIGHTMAGENTA
        FG_CYAN,                                            // LIGTHCYAN
        FG_WHITE                                            // WHITE
    };
    bool isBold = (color > LOGC_GREY && color < MAX_LOGCOLOR);
    fprintf(isError ? stderr : stdout, isBold ? "\x1b[%d;1m" : "\x1b[%dm", UnixColorFG[color]);
#endif
}

// static
void LogMgr::ResetConsoleColor(bool isError)
{
#if PLATFORM == PLATFORM_WINDOWS
    HANDLE hConsole = GetStdHandle(isError ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
#else
    fprintf(isError ? stderr : stdout, "\x1b[0m");
#endif
}

// static
uint32 LogMgr::WritePrefix(FILE* file, LogLevel level)
{
    switch (level) {
        case LOGL_CRASH: return fprintf(file, "CRASH ALERT: ");
        case LOGL_ERROR: return fprintf(file, "ERROR: ");
        default:
            break;
    }
    return 0;
}

// static
FILE* LogMgr::OpenFile(const std::string& path, bool isAppend)
{
    FILE* file = NULL;
    if (LogMgr::CreatePath(path))
        file = fopen(path.c_str(), isAppend ? "a+b" : "w");
    else
        sLogMgr->WriteConsoleLn(LOGL_ERROR, "LogMgr: unable to create path '%s'", path.c_str());
    return file;
}

void LogMgr::_InitColors(const std::string& colors)
{
    memset(_colors, 0, sizeof(LogColor) * MAX_LOGLEVEL);

    int color;
    std::istringstream ss(colors);
    for (uint8 i = 0; i < MAX_LOGLEVEL; ++i)
    {
        ss >> color;
        if (!ss)
            return;

        if (color < 0 || color >= MAX_LOGCOLOR)
            return;

        _colors[i] = LogColor(color);
        // At least one color is read - enough to use colors
        _useColor = true;
    }
}