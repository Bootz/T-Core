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

#ifndef TRILLIUMCORE_LOGMGR_H
#define TRILLIUMCORE_LOGMGR_H

#include "Common.h"

#include <ace/Singleton.h>

#define SERVER_LOG "Server"
#define DBERROR_LOG "DbErrors"
#define RA_LOG "RA"
#define CHAR_LOG "Char"
#define ARENA_LOG "Arena"
#define SQLDRIVER_LOG "SqlDriver"
#define SQLDEVELOPER_LOG "SqlDeveloper"
#define SOCKET_LOG "Socket"
#define GM_LOG "GM"

enum LogLevel
{
    LOGL_CRASH  = -2,
    LOGL_ERROR  = -1,

    LOGL_STRING = 0,
    LOGL_WARNING,
    LOGL_INFO,
    LOGL_FULL,

    MAX_LOGLEVEL
};

enum LogColor
{
    LOGC_BLACK = 0,
    LOGC_RED,
    LOGC_GREEN,
    LOGC_BROWN,
    LOGC_BLUE,
    LOGC_MAGENTA,
    LOGC_CYAN,
    LOGC_GREY,
    LOGC_YELLOW,
    LOGC_LIGHTRED,
    LOGC_LIGHTGREEN,
    LOGC_LIGHTBLUE,
    LOGC_LIGHTMAGENTA,
    LOGC_LIGHTCYAN,
    LOGC_WHITE,

    MAX_LOGCOLOR,

    LOGC_ERROR = LOGC_LIGHTRED
};

const uint32 LogFlushManual = uint32(-1);

class LogMgr
{
    friend class ACE_Singleton<LogMgr, ACE_Thread_Mutex>;
    LogMgr();
    ~LogMgr();

    class PhysicalLogFile
    {
    public:
        PhysicalLogFile(const std::string& dir, const std::string& fileName, const std::string& timeStampFmt, bool dateSplit, bool isAppend, uint32 flushBytes);
        ~PhysicalLogFile();

        void Flush();

        void UpdateSettings(const std::string& timeStampFmt, bool dateSplit, bool isAppend, uint32 flushBytes);
        void Write(LogLevel level, bool addNewLine, bool withTime, const std::string& msg);
        void Write(LogLevel level, bool addNewLine, bool withTime, const char* fmt, va_list& lst);

        void Open();
        void Close();

        uint8 IncreaseRefCount(uint8 amount = 1);
        uint8 DecreaseRefCount(uint8 amount = 1);

        const std::string& GetFileName() const { return _fileName; }
        const std::string& GetTimestampFmt() const { return _timeStampFmt; }
    private:
        void _CheckDate();

        std::string _dir;
        std::string _fileName;
        std::string _timeStampFmt;
        bool _dateSplit;
        bool _isAppend;
        uint32 _flushBytes;
        FILE* _file;
        ACE_Recursive_Thread_Mutex _lock;
        uint32 _writtenLength;
        uint8 _refCount;
        uint8 _lastDay;
    };

    class LogFile
    {
        friend class LogMgr;
    public:
        LogFile(const std::string& name, PhysicalLogFile* physicalLog, bool enabled, LogLevel level, bool consoleFlag, bool dbFlag);
        ~LogFile();

        void Flush();

        void Write(LogLevel level, bool addNewLine, bool withTime, const std::string& msg);
        void Write(LogLevel level, bool addNewLine, bool withTime, const char* fmt, va_list& lst);

        void WriteDb(LogLevel level, const std::string& msg);
        void WriteDb(LogLevel level, const char* fmt, va_list& lst);

        void SetLogLevel(LogLevel level) { _level = level; }
        LogLevel GetLogLevel() const { return _level; }
        bool ShouldLog(LogLevel level) const { return level <= _level; }

        void SetEnabled(bool enabled) { _enabled = enabled; }
        bool IsEnabled() const { return _enabled && _physicalLog; }

        void Close();

        PhysicalLogFile* GetPhysicalLog() const { return _physicalLog; }

    private:
        std::string _name;
        PhysicalLogFile* _physicalLog;
        bool _enabled;
        LogLevel _level;
        bool _consoleFlag;
        bool _dbFlag;
    };

    typedef UNORDERED_MAP <std::string, PhysicalLogFile*> PhysicalLogsMap;
    typedef UNORDERED_MAP <std::string, LogFile*> LogsMap;
    typedef std::vector<PhysicalLogFile*> PhysicalLogs;

public:
    static uint32 OutTimestamp(FILE* file, const std::string& timeStampFmt);
    static uint32 OutTimestamp(const std::string& path, const std::string& timeStampFmt);
    static bool CreatePath(const std::string& path);
    static void SetConsoleColor(bool isError, LogColor color);
    static void ResetConsoleColor(bool isError);
    static uint32 WritePrefix(FILE* file, LogLevel level);
    static FILE* OpenFile(const std::string& path, bool isAppend);

    void Initialize();
    void Clear();

    void WriteFile(const std::string& path, bool isAppend, const std::string& msg);
    void WriteFile(const std::string& path, bool isAppend, const char* fmt, va_list& lst);
    void WriteFile(const std::string& path, bool isAppend, const char* fmt, ...)    ATTR_PRINTF(4, 5);

    void WriteLn(const char* logName, const std::string& msg);
    void WriteLn(const char* logName, const char* fmt, va_list& lst);
    void WriteLn(const char* logName, const char* fmt, ...)                                 ATTR_PRINTF(3, 4);
    void WriteLn(const char* logName, LogLevel level, const std::string& msg);
    void WriteLn(const char* logName, LogLevel level, const char* fmt, va_list& lst);
    void WriteLn(const char* logName, LogLevel level, const char* fmt, ...)                 ATTR_PRINTF(4, 5);

    // Note: messages logged with Write methods are not written to DB.
    void Write(const char* logName, bool withTime, const std::string& msg);
    void Write(const char* logName, bool withTime, const char* fmt, va_list& lst);
    void Write(const char* logName, bool withTime, const char* fmt, ...)                    ATTR_PRINTF(4, 5);
    void Write(const char* logName, LogLevel level, bool withTime, const std::string& msg);
    void Write(const char* logName, LogLevel level, bool withTime, const char* fmt, va_list& lst);
    void Write(const char* logName, LogLevel level, bool withTime, const char* fmt, ...)    ATTR_PRINTF(5, 6);

    void WriteConsoleLn(LogLevel level, const std::string& msg) const;
    void WriteConsoleLn(LogLevel level, const char* fmt, va_list& lst) const;
    void WriteConsoleLn(LogLevel level, const char* fmt, ...) const                         ATTR_PRINTF(3, 4);

    void WriteConsole(LogLevel level, const std::string& msg) const;
    void WriteConsole(LogLevel level, const char* fmt, va_list& lst) const;
    void WriteConsole(LogLevel level, const char* fmt, ...) const                           ATTR_PRINTF(3, 4);
    
    void Flush(const char* logName);

    void RegisterLogFile(const char* logName);
    void UnregisterLogFile(const char* logName);

    void SetLogEnabled(const char* logName, bool enabled);
    bool IsLogEnabled(const char* logName) const;
    bool ToggleLogEnabled(const char* logName);

    const std::string& GetLogDirectory() const { return _dir; }
    
    void SetRealmId(uint32 realmId) { _realmId = realmId; }
    uint32 GetRealmId() const { return _realmId; }
 
    void ResetLogDb() { _logDb = false; }
    void SetLogDb();
    bool IsLogDb(LogLevel level) const { return _logDb && (level <= _logDbLevel); }

    bool IsDumpCharacters() const { return _dumpCharacters; }
    void WriteCharacterDump(uint32 accountId, uint32 guid, const char* name, const char* data);

    void WriteGmCommand(uint32 accountId, const char* fmt, ...)                             ATTR_PRINTF(3, 4);

    void SetLogLevel(const char* logName, uint32 level);
    void SetConsoleLogLevel(uint32 level);
    void SetDbLogLevel(uint32 level);
    bool ShouldLog(const char* logName, LogLevel level) const
    {
        bool res = level <= _logConsoleLevel;
        if (!res)
            if (LogFile const* log = _GetLog(logName))
                if (log->IsEnabled())
                    res = log->ShouldLog(level);
        return res;
    }

private:
    void _InitColors(const std::string& colors);

    void _Write(const char* logName, LogLevel level, bool appendNewLine, bool withTime, const std::string& msg);
    void _Write(const char* logName, LogLevel level, bool appendNewLine, bool withTime, const char* fmt, va_list& lst);

    void _WriteConsole(LogLevel level, bool appendNewLine, const std::string& msg) const;
    void _WriteConsole(LogLevel level, bool appendNewLine, const char* fmt, va_list& lst) const;
    
    LogFile* _GetLog(const char* logName)
    {
        LogsMap::iterator itr = _logsMap.find(logName);
        if (itr != _logsMap.end())
            return itr->second;
        return NULL;
    }

    LogFile const* _GetLog(const char* logName) const
    {
        LogsMap::const_iterator itr = _logsMap.find(logName);
        if (itr != _logsMap.end())
            return itr->second;
        return NULL;
    }

    PhysicalLogsMap _physicalLogsMap;
    PhysicalLogs _physicalLogs;
    LogsMap _logsMap;
    mutable ACE_Recursive_Thread_Mutex _consoleLock;

    std::string _dir;
    uint32 _realmId;
    bool _logDb;
    LogLevel _logDbLevel;
    LogLevel _logConsoleLevel;

    bool _dumpCharacters;
    bool _dumpCharactersSeparate;
    std::string _dumpCharactersDir;

    bool _useColor;
    LogColor _colors[MAX_LOGLEVEL];

    bool _logGmPerAccount;
    std::string _gmFilePath;
    std::string _gmTimestampFmt;
};

#define sLogMgr ACE_Singleton<LogMgr, ACE_Thread_Mutex>::instance()

#define LOG(...) do { sLogMgr->WriteLn(SERVER_LOG, LOGL_STRING, __VA_ARGS__); } while(0)

#define BASIC_LOG(...)                                              \
    do {                                                            \
        if (sLogMgr->ShouldLog(SERVER_LOG, LOGL_WARNING))           \
            sLogMgr->WriteLn(SERVER_LOG, LOGL_WARNING, __VA_ARGS__);\
    } while(0)

#define DETAIL_LOG(...)                                             \
    do {                                                            \
        if (sLogMgr->ShouldLog(SERVER_LOG, LOGL_INFO))              \
            sLogMgr->WriteLn(SERVER_LOG, LOGL_INFO, __VA_ARGS__);   \
    } while(0)

#define DEBUG_LOG(...)                                              \
    do {                                                            \
        if (sLogMgr->ShouldLog(SERVER_LOG, LOGL_FULL))              \
            sLogMgr->WriteLn(SERVER_LOG, LOGL_FULL, __VA_ARGS__);   \
    } while(0)

#endif