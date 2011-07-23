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

#include "LogMgr.h"
#include "Config.h"

LogMgr::LogFile::LogFile(const std::string& name) : _name(name)
{
    _fileName = sConfig->GetStringDefault(std::string("Log." + _name + ".File").c_str(), "");
    _dateSplit = sConfig->GetBoolDefault(std::string("Log." + _name + ".SplitDate").c_str(), true);
    _isAppend = sConfig->GetBoolDefault(std::string("Log." + _name + ".Append").c_str(), true);
    _consoleFlag = sConfig->GetBoolDefault(std::string("Log." + _name + ".Screen").c_str(), false);
    _flushBytes = sConfig->GetIntDefault(std::string("Log." + _name + ".FlushBytes").c_str(), 0);
}

LogMgr::LogFile::~LogFile()
{
    Close();
}

void LogMgr::LogFile::Close()
{
    sLogMgr->UnregisterLogFile(this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Physical log
LogMgr::PhysicalLogFile::PhysicalLogFile(const std::string& fileName, bool dateSplit, bool isAppend, uint32 flushBytes) 
    : _fileName(fileName), _dateSplit(dateSplit), _isAppend(isAppend), _flushBytes(flushBytes), _file(NULL), _writtenLength(0), _refCount(1)
{
    Open();
}

LogMgr::PhysicalLogFile::~PhysicalLogFile()
{
    Close();
}

uint8 LogMgr::PhysicalLogFile::IncreaseRefCount(uint8 amount)
{
    ACE_Guard<ACE_Thread_Mutex> guard(_lock);
    _refCount += amount;
    return _refCount;
}

uint8 LogMgr::PhysicalLogFile::DecreaseRefCount(uint8 amount)
{
    ACE_Guard<ACE_Thread_Mutex> guard(_lock);
    if (_refCount >= amount)
        _refCount -= amount;
    else
        _refCount = 0;
    return _refCount;
}

void LogMgr::PhysicalLogFile::Open()
{
    ACE_Guard<ACE_Thread_Mutex> guard(_lock);
    if (!_fileName.empty() && !_file)
    {
        std::string tmp(_fileName);
        if (_dateSplit)
        {
            time_t t = time(NULL);
            tm* aTm = localtime(&t);
            _lastDay = aTm->tm_mday;
            // TODO: Add more formatting strings and recursive dir creation if necessary
            // Replace $d with date value if applicable
            int dpos = tmp.find("$d");
            if (dpos != tmp.npos)
            {
                char szDate[12];
                sprintf(szDate, "%-4d-%02d-%02d", aTm->tm_year + 1900, aTm->tm_mon + 1, aTm->tm_mday);
                tmp.replace(dpos, 2, szDate, strlen(szDate));
            }
            else
                // TODO: Log!
                ;
        }
        _file = fopen(tmp.c_str(), _isAppend ? "a+b" : "w");
    }
}

void LogMgr::PhysicalLogFile::Close()
{
    ACE_Guard<ACE_Thread_Mutex> guard(_lock);
    if (_file) {
        fclose(_file);
        _file = NULL;
    }
}

void LogMgr::PhysicalLogFile::UpdateSettings(bool dateSplit, bool isAppend, uint32 flushBytes)
{
    // If someone specifies both to split and not to split dates, then prefer split
    _dateSplit |= dateSplit;
    // If someone specifies both to append or to overwrite file, then prefer append
    _isAppend |= isAppend;
    // Prefer minimum number of bytes to write
    _flushBytes = std::min(_flushBytes, flushBytes);
    // Increase reference count
    IncreaseRefCount();
}

void LogMgr::PhysicalLogFile::_CheckDate()
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

void LogMgr::PhysicalLogFile::Write(const char* fmt, va_list& lst)
{
    ACE_Guard<ACE_Thread_Mutex> guard(_lock);
    if (_file)
    {
        _CheckDate();
        // Show time
        LogMgr::OutTimestamp(_file);
        // Write message
        _writtenLength += vfprintf(_file, fmt, lst);
        _writtenLength +=  fprintf(_file, "\n");
        // Flush to disk if necessary amount of bytes were written
        if (_writtenLength >= _flushBytes)
        {
            fflush(_file);
            _writtenLength = 0;
        }
    }
}

// I know it is bad to copy/paste, but I'm too lazy now to invent something
void LogMgr::PhysicalLogFile::Write(const std::string& msg)
{
    ACE_Guard<ACE_Thread_Mutex> guard(_lock);
    if (_file)
    {
        _CheckDate();
        // Show time
        LogMgr::OutTimestamp(_file);
        // Write message
        _writtenLength += fprintf(_file, "%s\n", msg.c_str());
        // Flush to disk if necessary amount of bytes were written
        if (_writtenLength >= _flushBytes)
        {
            fflush(_file);
            _writtenLength = 0;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Log manager
LogMgr::LogMgr()
{
    Initialize();
}

LogMgr::~LogMgr()
{
    // Clear map
    _physicalLogsMap.clear();
    // Delete resources
    for (PhysicalLogs::iterator itr = _physicalLogs.begin(); itr != _physicalLogs.end(); ++itr)
        delete (*itr);
    _physicalLogs.clear();
    // Delete logical logs
    for (LogsMap::iterator itr = _logsMap.begin(); itr != _logsMap.end(); ++itr)
        delete itr->second;
    _logsMap.clear();
}

void LogMgr::Initialize()
{
    // Initialize all common logs here
    RegisterLogFile(new LogFile("AntiCheat"));
}

void LogMgr::RegisterLogFile(LogFile* logFile)
{
    PhysicalLogFile* log = NULL;
    // Check if there is already PhysicalLog for given file path
    for (PhysicalLogs::iterator itr = _physicalLogs.begin(); itr != _physicalLogs.end(); ++itr)
    {
        if ((*itr)->GetFileName() == logFile->_fileName)
        {
            // Yes, there is
            log = (*itr);
            log->UpdateSettings(logFile->_dateSplit, logFile->_isAppend, logFile->_flushBytes);
            break;
        }
    }
    // No, there is not
    if (!log)
    {
        log = new PhysicalLogFile(logFile->_fileName, logFile->_dateSplit, logFile->_isAppend, logFile->_flushBytes);
        _physicalLogs.push_back(log);
    }
    _physicalLogsMap[logFile->_name] = log;
    // Save log
    _logsMap[logFile->_name] = logFile;
    sLog->outString("LogMgr: log %s succesfully registered for file %s", logFile->_name.c_str(), logFile->_fileName.c_str());
}

void LogMgr::UnregisterLogFile(LogFile* logFile)
{
    // Find corresponding physical log
    if (PhysicalLogFile* log = _physicalLogsMap[logFile->_name])
    {
        // Decrease reference count
        // If no more references are found, delete object
        if (!log->DecreaseRefCount())
            delete log;
        // Remove information from log map
        _physicalLogsMap.erase(logFile->_name);
    }
    else
        // TODO: WTF?!
        ;
}

void LogMgr::Write(const std::string& logName, const std::string& msg)
{
    // Find corresponding physical log
    if (PhysicalLogFile* log = _physicalLogsMap[logName])
        log->Write(msg);
    else
        // TODO: WTF?!
        ;
}

void LogMgr::Write(const std::string& logName, const char* fmt, va_list& lst)
{
    // Find corresponding physical log
    if (PhysicalLogFile* log = _physicalLogsMap[logName])
        log->Write(fmt, lst);
    else
        // TODO: WTF?!
        ;
}

void LogMgr::Write(const std::string& logName, const char* fmt, ...)
{
    // Find corresponding physical log
    if (PhysicalLogFile* log = _physicalLogsMap[logName])
    {
        va_list lst;
        va_start(lst, fmt);
        log->Write(fmt, lst);
        va_end(lst);
    }
    else
        // TODO: WTF?!
        ;
}

// static
void LogMgr::OutTimestamp(FILE* file)
{
    time_t t = time(NULL);
    tm* aTm = localtime(&t);
    fprintf(file, "%-4d-%02d-%02d %02d:%02d:%02d ", aTm->tm_year + 1900, aTm->tm_mon + 1, aTm->tm_mday, aTm->tm_hour, aTm->tm_min, aTm->tm_sec);
} 