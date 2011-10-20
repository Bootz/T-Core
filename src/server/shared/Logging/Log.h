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

#ifndef TRILLIUMCORE_LOG_H
#define TRILLIUMCORE_LOG_H

#include "Common.h"

#include <ace/Null_Mutex.h>
#include <ace/Singleton.h>

enum DebugLogFilters
{
    LOG_FILTER_NONE                     = 0x00000000,
    LOG_FILTER_UNITS                    = 0x00000001,   // Anything related to units that doesn't fit in other categories. ie. creature formations
    LOG_FILTER_PETS                     = 0x00000002,
    LOG_FILTER_VEHICLES                 = 0x00000004,
    LOG_FILTER_TSCR                     = 0x00000008,   // C++ AI, instance scripts, etc.
    LOG_FILTER_DATABASE_AI              = 0x08000010,   // SmartAI, EventAI, CreatureAI
    LOG_FILTER_MAPSCRIPTS               = 0x00000020,
    LOG_FILTER_NETWORKIO                = 0x00000040,   // Anything packet/netcode related
    LOG_FILTER_SPELLS_AURAS             = 0x00000080,
    LOG_FILTER_ACHIEVEMENTSYS           = 0x00000100,
    LOG_FILTER_CONDITIONSYS             = 0x00000200,
    LOG_FILTER_POOLSYS                  = 0x00000400,
    LOG_FILTER_AUCTIONHOUSE             = 0x00000800,
    LOG_FILTER_BATTLEGROUND             = 0x00001000,   // Anything related to arena's and battlegrounds
    LOG_FILTER_OUTDOORPVP               = 0x00002000,
    LOG_FILTER_CHATSYS                  = 0x00004000,
    LOG_FILTER_LFG                      = 0x00008000,
    LOG_FILTER_MAPS                     = 0x00010000,   // Maps, instances, grids, cells, visibility
    LOG_FILTER_PLAYER_LOADING           = 0x00020000,   // Debug output from Player::_Load functions
    LOG_FILTER_PLAYER_ITEMS             = 0x00040000,   // Anything item related
    LOG_FILTER_PLAYER_SKILLS            = 0x00080000,   // Skills related
    LOG_FILTER_LOOT                     = 0x00100000,   // Loot related
    LOG_FILTER_GUILD                    = 0x00200000,   // Guild related
    LOG_FILTER_TRANSPORTS               = 0x00400000,   // Transport related
};

class Log
{
    friend class ACE_Singleton<Log, ACE_Thread_Mutex>;
    Log();
    ~Log();

    public:
        void outString(const char* fmt, ...)                    ATTR_PRINTF(2, 3);
        void outString();
        void outStringInLine(const char* fmt, ...)              ATTR_PRINTF(2, 3);
        void outError(const char* fmt, ...)                     ATTR_PRINTF(2, 3);
        void outCrash(const char* fmt, ...)                     ATTR_PRINTF(2, 3);
        void outBasic(const char* fmt, ...)                     ATTR_PRINTF(2, 3);
        void outDetail(const char* fmt, ...)                    ATTR_PRINTF(2, 3);
        void outSQLDev( const char* fmt, ...)                   ATTR_PRINTF(2, 3);
        void outDebug(DebugLogFilters f, const char* fmt, ...)  ATTR_PRINTF(3, 4);
        void outStaticDebug(const char* fmt, ...)               ATTR_PRINTF(2, 3);
        void outDebugInLine(const char* fmt, ...)               ATTR_PRINTF(2, 3);
        void outErrorDb(const char* fmt, ...)                   ATTR_PRINTF(2, 3);

        bool IsOutDebug() const;
    private:

        DebugLogFilters _DebugLogMask;
};

#define sLog ACE_Singleton<Log, ACE_Thread_Mutex>::instance()

#endif

