/*
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

class LogMgr
{
    class LogFile
    {
        friend class LogMgr;
    public:
        explicit LogFile(const std::string& name);
        ~LogFile();

        void Close();

    private:
        std::string _name;
        std::string _fileName;
        bool _dateSplit;
        bool _isAppend;
        bool _consoleFlag;
        uint32 _flushBytes;
        ACE_Thread_Mutex _lock;
    };

    class PhysicalLogFile
    {
    public:
        PhysicalLogFile(const std::string& fileName, bool dateSplit, bool isAppend, uint32 flushBytes);
        ~PhysicalLogFile();

        void UpdateSettings(bool dateSplit, bool isAppend, uint32 flushBytes);
        void Write(const std::string& msg);
        void Write(const char* fmt, va_list& lst);

        void Open();
        void Close();

        uint8 IncreaseRefCount(uint8 amount = 1);
        uint8 DecreaseRefCount(uint8 amount = 1);

        const std::string& GetFileName() const { return _fileName; }
    private:
        void _CheckDate();

        std::string _fileName;
        bool _dateSplit;
        bool _isAppend;
        uint32 _flushBytes;
        FILE* _file;
        ACE_Thread_Mutex _lock;
        uint32 _writtenLength;
        uint8 _refCount;
        uint8 _lastDay;
    };

    typedef UNORDERED_MAP <std::string, PhysicalLogFile*> PhysicalLogsMap;
    typedef UNORDERED_MAP <std::string, LogFile*> LogsMap;
    typedef std::vector<PhysicalLogFile*> PhysicalLogs;

    friend class ACE_Singleton<LogMgr, ACE_Thread_Mutex>;
    LogMgr();
    ~LogMgr();
public:
    static void OutTimestamp(FILE* file);

    void Initialize();

    void Write(const std::string& logName, const std::string& msg);
    void Write(const std::string& logName, const char* fmt, va_list& lst);
    void Write(const std::string& logName, const char* fmt, ...)     ATTR_PRINTF(3, 4);

private:
    void RegisterLogFile(LogFile* logFile);
    void UnregisterLogFile(LogFile* logFile);

    PhysicalLogsMap _physicalLogsMap;
    PhysicalLogs _physicalLogs;
    LogsMap _logsMap;
};

#define sLogMgr ACE_Singleton<LogMgr, ACE_Thread_Mutex>::instance()

#endif