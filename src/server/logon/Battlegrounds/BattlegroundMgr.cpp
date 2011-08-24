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

#include "Common.h"
#include "ObjectMgr.h"
#include "ArenaTeamMgr.h"
#include "World.h"
#include "WorldPacket.h"

#include "ArenaTeam.h"
#include "BattlegroundMgr.h"
#include "Chat.h"
#include "Map.h"
#include "MapInstanced.h"
#include "MapManager.h"
#include "Player.h"
#include "GameEventMgr.h"
#include "SharedDefines.h"
#include "Formulas.h"
#include "DisableMgr.h"

/*********************************************************/
/***            BATTLEGROUND MANAGER                   ***/
/*********************************************************/

BattlegroundMgr::BattlegroundMgr() : m_WeekResetTimeChecker(0), m_ArenaTesting(false)
{
    for (uint32 i = BATTLEGROUND_TYPE_NONE; i < MAX_BATTLEGROUND_TYPE_ID; i++)
        m_Battlegrounds[i].clear();
    m_NextRatingDiscardUpdate = sWorld->getIntConfig(CONFIG_ARENA_RATING_DISCARD_TIMER);
    m_Testing=false;
}

BattlegroundMgr::~BattlegroundMgr()
{
    DeleteAllBattlegrounds();
}

void BattlegroundMgr::DeleteAllBattlegrounds()
{
    for (uint32 i = BATTLEGROUND_TYPE_NONE; i < MAX_BATTLEGROUND_TYPE_ID; ++i)
    {
        for (BattlegroundSet::iterator itr = m_Battlegrounds[i].begin(); itr != m_Battlegrounds[i].end();)
        {
            Battleground* bg = itr->second;
            m_Battlegrounds[i].erase(itr++);
            if (!m_ClientBattlegroundIds[i][bg->GetBracketId()].empty())
                m_ClientBattlegroundIds[i][bg->GetBracketId()].erase(bg->GetClientInstanceID());
            delete bg;
        }
    }

    // destroy template battlegrounds that listed only in queues (other already terminated)
    for (uint32 bgTypeId = 0; bgTypeId < MAX_BATTLEGROUND_TYPE_ID; ++bgTypeId)
    {
        // ~Battleground call unregistring BG from queue
        while (!BGFreeSlotQueue[bgTypeId].empty())
            delete BGFreeSlotQueue[bgTypeId].front();
    }
}

// used to update running battlegrounds, and delete finished ones
void BattlegroundMgr::Update(uint32 diff)
{
    BattlegroundSet::iterator itr, next;
    for (uint32 i = BATTLEGROUND_TYPE_NONE; i < MAX_BATTLEGROUND_TYPE_ID; ++i)
    {
        itr = m_Battlegrounds[i].begin();
        // skip updating battleground template
        if (itr != m_Battlegrounds[i].end())
            ++itr;
        for (; itr != m_Battlegrounds[i].end(); itr = next)
        {
            next = itr;
            ++next;
            itr->second->Update(diff);
            // use the SetDeleteThis variable
            // direct deletion caused crashes
            if (itr->second->ToBeDeleted())
            {
                Battleground* bg = itr->second;
                m_Battlegrounds[i].erase(itr);
                if (!m_ClientBattlegroundIds[i][bg->GetBracketId()].empty())
                    m_ClientBattlegroundIds[i][bg->GetBracketId()].erase(bg->GetClientInstanceID());

                delete bg;
            }
        }
    }

    // update scheduled queues
    if (!m_QueueUpdateScheduler.empty())
    {
        std::vector<uint64> scheduled;
        {
            //copy vector and clear the other
            scheduled = std::vector<uint64>(m_QueueUpdateScheduler);
            m_QueueUpdateScheduler.clear();
            //release lock
        }

        for (uint8 i = 0; i < scheduled.size(); i++)
        {
            uint32 arenaMMRating = scheduled[i] >> 32;
            uint8 arenaType = scheduled[i] >> 24 & 255;
            BattlegroundQueueTypeId bgQueueTypeId = BattlegroundQueueTypeId(scheduled[i] >> 16 & 255);
            BattlegroundTypeId bgTypeId = BattlegroundTypeId((scheduled[i] >> 8) & 255);
            BattlegroundBracketId bracket_id = BattlegroundBracketId(scheduled[i] & 255);
            m_BattlegroundQueues[bgQueueTypeId].Update(bgTypeId, bracket_id, arenaType, arenaMMRating > 0, arenaMMRating);
        }
    }

    // if rating difference counts, maybe force-update queues
    if (sWorld->getIntConfig(CONFIG_ARENA_MAX_RATING_DIFFERENCE) && sWorld->getIntConfig(CONFIG_ARENA_RATING_DISCARD_TIMER))
    {
        // it's time to force update
        if (m_NextRatingDiscardUpdate < diff)
        {
            // forced update for rated arenas (scan all, but skipped non rated)
            sLog->outDebug(LOG_FILTER_BATTLEGROUND, "BattlegroundMgr: UPDATING ARENA QUEUES");
            for (int qtype = BATTLEGROUND_QUEUE_2v2; qtype <= BATTLEGROUND_QUEUE_5v5; ++qtype)
                for (int bracket = BG_BRACKET_ID_FIRST; bracket < MAX_BATTLEGROUND_BRACKETS; ++bracket)
                    m_BattlegroundQueues[qtype].Update(
                        BATTLEGROUND_AA, BattlegroundBracketId(bracket),
                        BattlegroundMgr::BGArenaType(BattlegroundQueueTypeId(qtype)), true, 0);

            m_NextRatingDiscardUpdate = sWorld->getIntConfig(CONFIG_ARENA_RATING_DISCARD_TIMER);
        }
        else
            m_NextRatingDiscardUpdate -= diff;
    }
    if (sWorld->getBoolConfig(CONFIG_AUTO_WEEK_RESET))
    {
        if (m_WeekResetTimeChecker < diff)
        {
            if (time(NULL) > m_NextWeekResetTime)
            {
                sArenaTeamMgr->WeekReset();
                m_NextWeekResetTime = m_NextWeekResetTime + BATTLEGROUND_ARENA_POINT_DISTRIBUTION_DAY * 7;
                sWorld->setWorldState(WS_ARENA_DISTRIBUTION_TIME, uint64(m_NextWeekResetTime));
            }
            m_WeekResetTimeChecker = 600000; // check 10 minutes
        }
        else
            m_WeekResetTimeChecker -= diff;
    }
}

void BattlegroundMgr::BuildBattlegroundStatusPacket(WorldPacket *data, Battleground *bg, uint8 QueueSlot, uint8 StatusID, uint32 Time1, uint32 Time2, uint8 arenatype, uint8 uiFrame)
{
    if (!bg)
        StatusID = STATUS_NONE;

    switch (StatusID)
    {
        case STATUS_NONE:
        {
            // STATUS1 resets clients' BG Info
            data->Initialize(SMSG_BATTLEFIELD_STATUS1, 4);
            *data << uint32(QueueSlot);                         // queue id (0...1)
        }
        break;
        case STATUS_WAIT_QUEUE:
        {
            // The client will set STATUS_WAIT_QUEUE at BGInfo once it receives this packet
            data->Initialize(SMSG_JOINED_BATTLEGROUND_QUEUE, (1+1+4+4+4+1+8+1+4));
            *data << uint8(0x20); // packed flag, seems to be always 0x20 for non-rated non-arena bgs
            *data << uint8(bg->GetMaxLevel()); // max level
            *data << uint32(Time1); // avg wait time
            *data << uint32(QueueSlot); // queueSlot
            *data << uint32(bg->GetClientInstanceID()); // instanceid
            *data << uint8(bg->GetMinLevel()); // lowest level (seems to be set to 0 even though its not 0 sometimes O.O)

            // packed uint64 (seems to be BG GUID)
            *data << uint32(bg->GetTypeID()); // BGTypeID
            *data << uint32(arenatype); // On retail 0x101F is sent here, but we need this value to be returned in PORT opcode
            // end

            *data << uint8(0); // teamsize, 0 if not arena
            *data << uint32(Time2); // time in queue
        }
        break;
        case STATUS_WAIT_JOIN:
        {
            // The client will set STATUS_WAIT_JOIN at BGInfo once it receives this packet
            data->Initialize(SMSG_BATTLEFIELD_STATUS3, (1+1+4+8+4+1+4+4+1));
            *data << uint8(bg->isRated() ? 128 : 0); // isarena?
            *data << uint8(bg->GetMinLevel());
            *data << uint32(bg->GetClientInstanceID()); // instance id

            *data << uint32(bg->GetTypeID()); // BGTypeID
            *data << uint32(arenatype);

            *data << uint32(QueueSlot); // queueslot
            *data << uint8(arenatype); // teamsize (0 if not arena)
            *data << uint32(Time1); // port expiration time

            if(bg->GetTypeID() != BATTLEGROUND_RB)
                *data << uint32(bg->GetMapId()); // mapid
            else *data << uint32(0);

            *data << uint8(bg->GetMaxLevel()); // highestLevel
        }
        break;
        case STATUS_IN_PROGRESS:
        {
            data->Initialize(SMSG_BATTLEFIELD_STATUS2, 100);
            *data << uint8(bg->isRated() ? 128 : 0);
            *data << uint32(Time2); // 
            *data << uint32(QueueSlot); // queueslot 
            *data << uint32(bg->GetMapId()); // MapID

            // This is bg guid
            *data << uint32(bg->GetTypeID()); // BGTypeID
            *data << uint16(0);
            *data << uint8(0x10); // High guid
            *data << uint8(0x1F); // High guid
            // end

            *data << uint32(Time1); // Time until BG closed
            *data << uint8(arenatype); // teamsize (0 if not arena)
            *data << uint8(bg->GetMaxLevel());
            *data << uint32(bg->GetClientInstanceID()); // instanceid
            *data << uint8(bg->GetMinLevel());        
        }
        break;
        case STATUS_WAIT_LEAVE:
        {
            // Not used...
            data->Initialize(SMSG_BATTLEFIELD_STATUS4, (1+4+1+1+1+4+1+4+1+4+1+8+1));
            *data << uint8(0); // flag
            *data << uint32(Time1); // 
            *data << uint8(bg->GetMinLevel()); // lowestLevel
            *data << uint8(0);
            *data << uint8(0);
            *data << uint32(QueueSlot); // queueSlot
            *data << uint8(bg->GetMaxLevel()); // highestLevel
            *data << uint32(Time2); //
            *data << uint8(0); // teamsize (0 if not arena)
            *data << uint32(bg->GetClientInstanceID()); // instanceid
            *data << uint8(0); 

            // This is bg guid
            *data << uint32(bg->GetTypeID()); // BGTypeID
            *data << uint16(0);
            *data << uint8(0x10); // High guid
            *data << uint8(0x1F); // High guid
            // end

            *data << uint8(0);
        }
        break;
    }
}

void BattlegroundMgr::BuildPvpLogDataPacket(WorldPacket *data, Battleground *bg)
{}

void BattlegroundMgr::BuildGroupJoinedBattlegroundPacket(WorldPacket *data, GroupJoinBattlegroundResult result)
{
    data->Initialize(SMSG_GROUP_JOINED_BATTLEGROUND, 4);
    *data << int32(result);
    if (result == ERR_BATTLEGROUND_JOIN_TIMED_OUT || result == ERR_BATTLEGROUND_JOIN_FAILED)
        *data << uint64(0);                                 // player guid
}

void BattlegroundMgr::BuildUpdateWorldStatePacket(WorldPacket *data, uint32 field, uint32 value)
{
    data->Initialize(SMSG_UPDATE_WORLD_STATE, 4+4);
    *data << uint32(field);
    *data << uint32(value);
}

void BattlegroundMgr::BuildPlaySoundPacket(WorldPacket *data, uint32 soundid)
{
    data->Initialize(SMSG_PLAY_SOUND, 4);
    *data << uint32(soundid);
}

void BattlegroundMgr::BuildPlayerLeftBattlegroundPacket(WorldPacket *data, uint64 guid)
{
    data->Initialize(SMSG_BATTLEGROUND_PLAYER_LEFT, 8);
    *data << uint64(guid);
}

void BattlegroundMgr::BuildPlayerJoinedBattlegroundPacket(WorldPacket *data, Player *plr)
{
    data->Initialize(SMSG_BATTLEGROUND_PLAYER_JOINED, 8);
    *data << uint64(plr->GetGUID());
}

Battleground* BattlegroundMgr::GetBattlegroundThroughClientInstance(uint32 instanceId, BattlegroundTypeId bgTypeId)
{
    //cause at HandleBattlegroundJoinOpcode the clients sends the instanceid he gets from
    //SMSG_BATTLEFIELD_LIST we need to find the battleground with this clientinstance-id
    Battleground* bg = GetBattlegroundTemplate(bgTypeId);
    if (!bg)
        return NULL;

    if (bg->isArena())
        return GetBattleground(instanceId, bgTypeId);

    for (BattlegroundSet::iterator itr = m_Battlegrounds[bgTypeId].begin(); itr != m_Battlegrounds[bgTypeId].end(); ++itr)
    {
        if (itr->second->GetClientInstanceID() == instanceId)
            return itr->second;
    }
    return NULL;
}

Battleground* BattlegroundMgr::GetBattleground(uint32 InstanceID, BattlegroundTypeId bgTypeId)
{
    if (!InstanceID)
        return NULL;
    //search if needed
    BattlegroundSet::iterator itr;
    if (bgTypeId == BATTLEGROUND_TYPE_NONE)
    {
        for (uint32 i = BATTLEGROUND_AV; i < MAX_BATTLEGROUND_TYPE_ID; i++)
        {
            itr = m_Battlegrounds[i].find(InstanceID);
            if (itr != m_Battlegrounds[i].end())
                return itr->second;
        }
        return NULL;
    }
    itr = m_Battlegrounds[bgTypeId].find(InstanceID);
    return ((itr != m_Battlegrounds[bgTypeId].end()) ? itr->second : NULL);
}

Battleground* BattlegroundMgr::GetBattlegroundTemplate(BattlegroundTypeId bgTypeId)
{
    //map is sorted and we can be sure that lowest instance id has only BG template
    return m_Battlegrounds[bgTypeId].empty() ? NULL : m_Battlegrounds[bgTypeId].begin()->second;
}

uint32 BattlegroundMgr::CreateClientVisibleInstanceId(BattlegroundTypeId bgTypeId, BattlegroundBracketId bracket_id)
{
    if (IsArenaType(bgTypeId))
        return 0;                                           //arenas don't have client-instanceids

    // we create here an instanceid, which is just for
    // displaying this to the client and without any other use..
    // the client-instanceIds are unique for each battleground-type
    // the instance-id just needs to be as low as possible, beginning with 1
    // the following works, because std::set is default ordered with "<"
    // the optimalization would be to use as bitmask std::vector<uint32> - but that would only make code unreadable
    uint32 lastId = 0;
    for (std::set<uint32>::iterator itr = m_ClientBattlegroundIds[bgTypeId][bracket_id].begin(); itr != m_ClientBattlegroundIds[bgTypeId][bracket_id].end();)
    {
        if ((++lastId) != *itr)                             //if there is a gap between the ids, we will break..
            break;
        lastId = *itr;
    }
    m_ClientBattlegroundIds[bgTypeId][bracket_id].insert(lastId + 1);
    return lastId + 1;
}

// create a new battleground that will really be used to play
Battleground* BattlegroundMgr::CreateNewBattleground(BattlegroundTypeId bgTypeId, PvPDifficultyEntry const* bracketEntry, uint8 arenaType, bool isRated)
{
    return NULL;
}

// used to create the BG templates
uint32 BattlegroundMgr::CreateBattleground(CreateBattlegroundData& data)
{
    return data.bgTypeId;
}

void BattlegroundMgr::CreateInitialBattlegrounds()
{
    uint32 oldMSTime = getMSTime();

    uint8 selectionWeight;
    BattlemasterListEntry const *bl;

    //                                               0   1                  2                  3       4       5                 6               7              8            9       10
    QueryResult result = WorldDatabase.Query("SELECT id, MinPlayersPerTeam, MaxPlayersPerTeam, MinLvl, MaxLvl, AllianceStartLoc, AllianceStartO, HordeStartLoc, HordeStartO, Weight, ScriptName FROM battleground_template");

    if (!result)
    {
        sLog->outErrorDb(">> Loaded 0 battlegrounds. DB table `battleground_template` is empty.");
        sLog->outString();
        return;
    }

    uint32 count = 0, startId;

    do
    {
        Field *fields = result->Fetch();

        uint32 bgTypeID_ = fields[0].GetUInt32();
        if (sDisableMgr->IsDisabledFor(DISABLE_TYPE_BATTLEGROUND, bgTypeID_, NULL))
            continue;

        // can be overwrite by values from DB
        bl = sBattlemasterListStore.LookupEntry(bgTypeID_);
        if (!bl)
        {
            sLog->outError("Battleground ID %u not found in BattlemasterList.dbc. Battleground not created.", bgTypeID_);
            continue;
        }

        CreateBattlegroundData data;
        data.bgTypeId = BattlegroundTypeId(bgTypeID_);
        data.IsArena = (bl->type == TYPE_ARENA);
        data.MinPlayersPerTeam = fields[1].GetUInt32();
        data.MaxPlayersPerTeam = fields[2].GetUInt32();
        data.LevelMin = fields[3].GetUInt32();
        data.LevelMax = fields[4].GetUInt32();
        //check values from DB
        if (data.MaxPlayersPerTeam == 0 || data.MinPlayersPerTeam == 0 || data.MinPlayersPerTeam > data.MaxPlayersPerTeam)
        {
            data.MinPlayersPerTeam = 0;                          // by default now expected strong full bg requirement
            data.MaxPlayersPerTeam = 40;
        }
        if (data.LevelMin == 0 || data.LevelMax == 0 || data.LevelMin > data.LevelMax)
        {
            //TO-DO: FIX ME
            data.LevelMin = 0;//bl->minlvl;
            data.LevelMax = 80;//bl->maxlvl;
        }

        startId = fields[5].GetUInt32();
        if (WorldSafeLocsEntry const* start = sWorldSafeLocsStore.LookupEntry(startId))
        {
            data.Team1StartLocX = start->x;
            data.Team1StartLocY = start->y;
            data.Team1StartLocZ = start->z;
            data.Team1StartLocO = fields[6].GetFloat();
        }
        else if (data.bgTypeId == BATTLEGROUND_AA || data.bgTypeId == BATTLEGROUND_RB)
        {
            data.Team1StartLocX = 0;
            data.Team1StartLocY = 0;
            data.Team1StartLocZ = 0;
            data.Team1StartLocO = fields[6].GetFloat();
        }
        else
        {
            sLog->outErrorDb("Table `battleground_template` for id %u have non-existed WorldSafeLocs.dbc id %u in field `AllianceStartLoc`. BG not created.", data.bgTypeId, startId);
            continue;
        }

        startId = fields[7].GetUInt32();
        if (WorldSafeLocsEntry const* start = sWorldSafeLocsStore.LookupEntry(startId))
        {
            data.Team2StartLocX = start->x;
            data.Team2StartLocY = start->y;
            data.Team2StartLocZ = start->z;
            data.Team2StartLocO = fields[8].GetFloat();
        }
        else if (data.bgTypeId == BATTLEGROUND_AA || data.bgTypeId == BATTLEGROUND_RB)
        {
            data.Team2StartLocX = 0;
            data.Team2StartLocY = 0;
            data.Team2StartLocZ = 0;
            data.Team2StartLocO = fields[8].GetFloat();
        }
        else
        {
            sLog->outErrorDb("Table `battleground_template` for id %u have non-existed WorldSafeLocs.dbc id %u in field `HordeStartLoc`. BG not created.", data.bgTypeId, startId);
            continue;
        }

        selectionWeight = fields[9].GetUInt8();
        data.scriptId = sObjectMgr->GetScriptId(fields[10].GetCString());
        data.BattlegroundName = bl->name[sWorld->GetDefaultDbcLocale()];
        data.MapID = bl->mapid[0];

        if (!CreateBattleground(data))
            continue;

        if (data.IsArena)
        {
            if (data.bgTypeId != BATTLEGROUND_AA)
                m_ArenaSelectionWeights[data.bgTypeId] = selectionWeight;
        }
        else if (data.bgTypeId != BATTLEGROUND_RB)
            m_BGSelectionWeights[data.bgTypeId] = selectionWeight;
        ++count;
    }
    while (result->NextRow());

    sLog->outString(">> Loaded %u battlegrounds in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    sLog->outString();
}

void BattlegroundMgr::BuildBattlegroundListPacket(WorldPacket *data, uint64 guid, Player* plr, BattlegroundTypeId bgTypeId, uint8 fromWhere)
{
    if (!plr)
        return;

    uint32 winner_kills = plr->GetRandomWinner() ? BG_REWARD_WINNER_HONOR_LAST : BG_REWARD_WINNER_HONOR_FIRST;
    uint32 winner_arena = plr->GetRandomWinner() ? BG_REWARD_WINNER_ARENA_LAST : BG_REWARD_WINNER_ARENA_FIRST;
    uint32 loser_kills = plr->GetRandomWinner() ? BG_REWARD_LOSER_HONOR_LAST : BG_REWARD_LOSER_HONOR_FIRST;

    winner_kills = Trillium::Honor::hk_honor_at_level(plr->getLevel(), float(winner_kills));
    loser_kills = Trillium::Honor::hk_honor_at_level(plr->getLevel(), float(loser_kills));

    data->Initialize(SMSG_BATTLEFIELD_LIST);
    *data << uint64(guid);                                  // battlemaster guid
    *data << uint8(fromWhere);                              // from where you joined
    *data << uint32(bgTypeId);                              // battleground id
    *data << uint8(0);                                      // unk
    *data << uint8(0);                                      // unk

    // Rewards
    *data << uint8(plr->GetRandomWinner());               // 3.3.3 hasWin
    *data << uint32(winner_kills);                           // 3.3.3 winHonor
    *data << uint32(winner_arena);                           // 3.3.3 winArena
    *data << uint32(loser_kills);                          // 3.3.3 lossHonor

    uint8 isRandom = bgTypeId == BATTLEGROUND_RB;

    *data << uint8(isRandom);                               // 3.3.3 isRandom
    if (isRandom)
    {
        // Rewards (random)
        *data << uint8(plr->GetRandomWinner());           // 3.3.3 hasWin_Random
        *data << uint32(winner_kills);                       // 3.3.3 winHonor_Random
        *data << uint32(winner_arena);                       // 3.3.3 winArena_Random
        *data << uint32(loser_kills);                      // 3.3.3 lossHonor_Random
    }

    if (bgTypeId == BATTLEGROUND_AA)                         // arena
    {
        *data << uint32(0);                                 // unk (count?)
    }
    else                                                    // battleground
    {
        size_t count_pos = data->wpos();
        *data << uint32(0);                                 // number of bg instances

        if (Battleground* bgTemplate = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId))
        {
            // expected bracket entry
            if (PvPDifficultyEntry const* bracketEntry = GetBattlegroundBracketByLevel(bgTemplate->GetMapId(), plr->getLevel()))
            {
                uint32 count = 0;
                BattlegroundBracketId bracketId = bracketEntry->GetBracketId();
                for (std::set<uint32>::iterator itr = m_ClientBattlegroundIds[bgTypeId][bracketId].begin(); itr != m_ClientBattlegroundIds[bgTypeId][bracketId].end();++itr)
                {
                    *data << uint32(*itr);
                    ++count;
                }
                data->put<uint32>(count_pos , count);
            }
        }
    }
}

void BattlegroundMgr::SendToBattleground(Player *pl, uint32 instanceId, BattlegroundTypeId bgTypeId)
{
    Battleground *bg = GetBattleground(instanceId, bgTypeId);
    if (bg)
    {
        uint32 mapid = bg->GetMapId();
        float x, y, z, O;
        uint32 team = pl->GetBGTeam();
        if (team == 0)
            team = pl->GetTeam();
        bg->GetTeamStartLoc(team, x, y, z, O);

        sLog->outDetail("BATTLEGROUND: Sending %s to map %u, X %f, Y %f, Z %f, O %f", pl->GetName(), mapid, x, y, z, O);
        pl->TeleportTo(mapid, x, y, z, O);
    }
    else
    {
        sLog->outError("player %u is trying to port to non-existent bg instance %u", pl->GetGUIDLow(), instanceId);
    }
}

void BattlegroundMgr::SendAreaSpiritHealerQueryOpcode(Player *pl, Battleground *bg, uint64 guid)
{
    WorldPacket data(SMSG_AREA_SPIRIT_HEALER_TIME, 12);
    uint32 time_ = 30000 - bg->GetLastResurrectTime();      // resurrect every 30 seconds
    if (time_ == uint32(-1))
        time_ = 0;
    data << guid << time_;
    pl->GetSession()->SendPacket(&data);
}

bool BattlegroundMgr::IsArenaType(BattlegroundTypeId bgTypeId)
{
    return (bgTypeId == BATTLEGROUND_AA ||
        bgTypeId == BATTLEGROUND_BE ||
        bgTypeId == BATTLEGROUND_NA ||
        bgTypeId == BATTLEGROUND_RV ||
        bgTypeId == BATTLEGROUND_RL ||
        bgTypeId == BATTLEGROUND_DS);
}

BattlegroundQueueTypeId BattlegroundMgr::BGQueueTypeId(BattlegroundTypeId bgTypeId, uint8 arenaType)
{
    switch(bgTypeId)
    {
        case BATTLEGROUND_WS:
            return BATTLEGROUND_QUEUE_WS;
        case BATTLEGROUND_AB:
            return BATTLEGROUND_QUEUE_AB;
        case BATTLEGROUND_AV:
            return BATTLEGROUND_QUEUE_AV;
        case BATTLEGROUND_EY:
            return BATTLEGROUND_QUEUE_EY;
        case BATTLEGROUND_SA:
            return BATTLEGROUND_QUEUE_SA;
        case BATTLEGROUND_IC:
            return BATTLEGROUND_QUEUE_IC;
        case BATTLEGROUND_RB:
            return BATTLEGROUND_QUEUE_RB;
        case BATTLEGROUND_AA:
        case BATTLEGROUND_NA:
        case BATTLEGROUND_RL:
        case BATTLEGROUND_BE:
        case BATTLEGROUND_DS:
        case BATTLEGROUND_RV:
            switch(arenaType)
            {
                case ARENA_TYPE_2v2:
                    return BATTLEGROUND_QUEUE_2v2;
                case ARENA_TYPE_3v3:
                    return BATTLEGROUND_QUEUE_3v3;
                case ARENA_TYPE_5v5:
                    return BATTLEGROUND_QUEUE_5v5;
                default:
                    return BATTLEGROUND_QUEUE_NONE;
            }
        default:
            return BATTLEGROUND_QUEUE_NONE;
    }
}

BattlegroundTypeId BattlegroundMgr::BGTemplateId(BattlegroundQueueTypeId bgQueueTypeId)
{
    switch(bgQueueTypeId)
    {
        case BATTLEGROUND_QUEUE_WS:
            return BATTLEGROUND_WS;
        case BATTLEGROUND_QUEUE_AB:
            return BATTLEGROUND_AB;
        case BATTLEGROUND_QUEUE_AV:
            return BATTLEGROUND_AV;
        case BATTLEGROUND_QUEUE_EY:
            return BATTLEGROUND_EY;
        case BATTLEGROUND_QUEUE_SA:
            return BATTLEGROUND_SA;
        case BATTLEGROUND_QUEUE_IC:
            return BATTLEGROUND_IC;
        case BATTLEGROUND_QUEUE_RB:
            return BATTLEGROUND_RB;
        case BATTLEGROUND_QUEUE_2v2:
        case BATTLEGROUND_QUEUE_3v3:
        case BATTLEGROUND_QUEUE_5v5:
            return BATTLEGROUND_AA;
        default:
            return BattlegroundTypeId(0);                   // used for unknown template (it existed and do nothing)
    }
}

uint8 BattlegroundMgr::BGArenaType(BattlegroundQueueTypeId bgQueueTypeId)
{
    switch(bgQueueTypeId)
    {
        case BATTLEGROUND_QUEUE_2v2:
            return ARENA_TYPE_2v2;
        case BATTLEGROUND_QUEUE_3v3:
            return ARENA_TYPE_3v3;
        case BATTLEGROUND_QUEUE_5v5:
            return ARENA_TYPE_5v5;
        default:
            return 0;
    }
}

void BattlegroundMgr::ToggleTesting()
{
    m_Testing = !m_Testing;
    if (m_Testing)
        sWorld->SendWorldText(LANG_DEBUG_BG_ON);
    else
        sWorld->SendWorldText(LANG_DEBUG_BG_OFF);
}

void BattlegroundMgr::ToggleArenaTesting()
{
    m_ArenaTesting = !m_ArenaTesting;
    if (m_ArenaTesting)
        sWorld->SendWorldText(LANG_DEBUG_ARENA_ON);
    else
        sWorld->SendWorldText(LANG_DEBUG_ARENA_OFF);
}

void BattlegroundMgr::SetHolidayWeekends(uint32 mask)
{
    for (uint32 bgtype = 1; bgtype < MAX_BATTLEGROUND_TYPE_ID; ++bgtype)
    {
        if (Battleground* bg = GetBattlegroundTemplate(BattlegroundTypeId(bgtype)))
        {
            bg->SetHoliday(mask & (1 << bgtype));
        }
    }
}

void BattlegroundMgr::ScheduleQueueUpdate(uint32 arenaMatchmakerRating, uint8 arenaType, BattlegroundQueueTypeId bgQueueTypeId, BattlegroundTypeId bgTypeId, BattlegroundBracketId bracket_id)
{
    //This method must be atomic, TODO add mutex
    //we will use only 1 number created of bgTypeId and bracket_id
    uint64 schedule_id = ((uint64)arenaMatchmakerRating << 32) | (arenaType << 24) | (bgQueueTypeId << 16) | (bgTypeId << 8) | bracket_id;
    bool found = false;
    for (uint8 i = 0; i < m_QueueUpdateScheduler.size(); i++)
    {
        if (m_QueueUpdateScheduler[i] == schedule_id)
        {
            found = true;
            break;
        }
    }
    if (!found)
        m_QueueUpdateScheduler.push_back(schedule_id);
}

uint32 BattlegroundMgr::GetMaxRatingDifference() const
{
    // this is for stupid people who can't use brain and set max rating difference to 0
    uint32 diff = sWorld->getIntConfig(CONFIG_ARENA_MAX_RATING_DIFFERENCE);
    if (diff == 0)
        diff = 5000;
    return diff;
}

uint32 BattlegroundMgr::GetRatingDiscardTimer() const
{
    return sWorld->getIntConfig(CONFIG_ARENA_RATING_DISCARD_TIMER);
}

uint32 BattlegroundMgr::GetPrematureFinishTime() const
{
    return sWorld->getIntConfig(CONFIG_BATTLEGROUND_PREMATURE_FINISH_TIMER);
}

void BattlegroundMgr::LoadBattleMastersEntry()
{
    uint32 oldMSTime = getMSTime();

    mBattleMastersMap.clear();                                  // need for reload case

    QueryResult result = WorldDatabase.Query("SELECT entry, bg_template FROM battlemaster_entry");

    if (!result)
    {
        sLog->outString(">> Loaded 0 battlemaster entries. DB table `battlemaster_entry` is empty!");
        sLog->outString();
        return;
    }

    uint32 count = 0;

    do
    {
        ++count;

        Field *fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();
        uint32 bgTypeId  = fields[1].GetUInt32();
        if (!sBattlemasterListStore.LookupEntry(bgTypeId))
        {
            sLog->outErrorDb("Table `battlemaster_entry` contain entry %u for not existed battleground type %u, ignored.", entry, bgTypeId);
            continue;
        }

        mBattleMastersMap[entry] = BattlegroundTypeId(bgTypeId);

    }
    while (result->NextRow());

    sLog->outString(">> Loaded %u battlemaster entries in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    sLog->outString();
}

HolidayIds BattlegroundMgr::BGTypeToWeekendHolidayId(BattlegroundTypeId bgTypeId)
{
    switch (bgTypeId)
    {
        case BATTLEGROUND_AV: return HOLIDAY_CALL_TO_ARMS_AV;
        case BATTLEGROUND_EY: return HOLIDAY_CALL_TO_ARMS_EY;
        case BATTLEGROUND_WS: return HOLIDAY_CALL_TO_ARMS_WS;
        case BATTLEGROUND_SA: return HOLIDAY_CALL_TO_ARMS_SA;
        case BATTLEGROUND_AB: return HOLIDAY_CALL_TO_ARMS_AB;
        case BATTLEGROUND_IC: return HOLIDAY_CALL_TO_ARMS_IC;
        default: return HOLIDAY_NONE;
    }
}

BattlegroundTypeId BattlegroundMgr::WeekendHolidayIdToBGType(HolidayIds holiday)
{
    switch (holiday)
    {
        case HOLIDAY_CALL_TO_ARMS_AV: return BATTLEGROUND_AV;
        case HOLIDAY_CALL_TO_ARMS_EY: return BATTLEGROUND_EY;
        case HOLIDAY_CALL_TO_ARMS_WS: return BATTLEGROUND_WS;
        case HOLIDAY_CALL_TO_ARMS_SA: return BATTLEGROUND_SA;
        case HOLIDAY_CALL_TO_ARMS_AB: return BATTLEGROUND_AB;
        case HOLIDAY_CALL_TO_ARMS_IC: return BATTLEGROUND_IC;
        default: return BATTLEGROUND_TYPE_NONE;
    }
}

bool BattlegroundMgr::IsBGWeekend(BattlegroundTypeId bgTypeId)
{
    return IsHolidayActive(BGTypeToWeekendHolidayId(bgTypeId));
}
