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

#include "Creature.h"
#include "CreatureGroups.h"
#include "ObjectMgr.h"

#include "CreatureAI.h"

#define MAX_DESYNC 5.0f

CreatureGroupInfoType   CreatureGroupMap;
CreatureGroupDataType   CreatureGroupDataMap;

void CreatureGroupManager::AddCreatureToGroup(uint32 groupId, Creature* member)
{
    Map *map = member->FindMap();
    if (!map)
        return;

    CreatureGroupHolderType::iterator itr = map->CreatureGroupHolder.find(groupId);

    //Add member to an existing group
    if (itr != map->CreatureGroupHolder.end())
    {
        sLog->outDebug(LOG_FILTER_UNITS, "Group found: %u, inserting creature GUID: %u, Group InstanceID %u", groupId, member->GetGUIDLow(), member->GetInstanceId());
        itr->second->AddMember(member);
    }
    //Create new group
    else
    {
        sLog->outDebug(LOG_FILTER_UNITS, "Group not found: %u. Creating new group.", groupId);
        CreatureGroup* group = new CreatureGroup(groupId);
        map->CreatureGroupHolder[groupId] = group;
        group->AddMember(member);
    }
}

void CreatureGroupManager::RemoveCreatureFromGroup(CreatureGroup* group, Creature* member)
{
    sLog->outDebug(LOG_FILTER_UNITS, "Deleting member pointer to GUID: %u from group %u", group->GetId(), member->GetDBTableGUIDLow());
    group->RemoveMember(member);

    if (group->isEmpty())
    {
        Map *map = member->FindMap();
        if (!map)
            return;

        sLog->outDebug(LOG_FILTER_UNITS, "Deleting group with InstanceID %u", member->GetInstanceId());
        map->CreatureGroupHolder.erase(group->GetId());
        delete group;
    }
}

void CreatureGroupManager::LoadCreatureGroups()
{
    uint32 oldMSTime = getMSTime();

    for (CreatureGroupInfoType::iterator itr = CreatureGroupMap.begin(); itr != CreatureGroupMap.end(); ++itr) // for reload case
        delete itr->second;
    CreatureGroupMap.clear();

    CreatureGroupDataMap.clear();

    //Check if groups without member exist
    PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_LOAD_CREGROUPS_WITHOUT_DATA);
    PreparedQueryResult result = WorldDatabase.Query(stmt);

    if (result)
        sLog->outDetail(">> %u Groups without member found, groups skipped.",result->Fetch()->GetInt32());

    //Check if member without group exist
    stmt = WorldDatabase.GetPreparedStatement(WORLD_LOAD_CREGROUP_DATA_WITHOUT_GROUP);
    result = WorldDatabase.Query(stmt);

    if (result)
        sLog->outDetail(">> %u Member without group found, member skipped.",result->Fetch()->GetInt32());

    //Get groups
    stmt = WorldDatabase.GetPreparedStatement(WORLD_LOAD_CREGROUPS);
    PreparedQueryResult result_data = WorldDatabase.Query(stmt);

    if (!result_data)
    {
        sLog->outString(">> Loaded 0 creature groups. DB table `creature_groups` is empty.");
            sLog->outString();
        return;
    }

    //Get member
    stmt = WorldDatabase.GetPreparedStatement(WORLD_LOAD_CREGROUP_DATA);
    PreparedQueryResult result_member = WorldDatabase.Query(stmt);

    if (!result_member)
    {
        sLog->outString(">> Loaded 0 creature groups. DB table `creature_group_data` is empty.");
        sLog->outString();
        return;
    }    

    std::set<uint32> guidSet;

    stmt = WorldDatabase.GetPreparedStatement(WORLD_LOAD_CREATURE_GUIDS);
    PreparedQueryResult guidResult = WorldDatabase.Query(stmt);

    if (guidResult)
    {
        do
        {
            Field* fields = guidResult->Fetch();
            uint32 guid = fields[0].GetUInt32();

            guidSet.insert(guid);

        }
        while (guidResult->NextRow());
    }

    //Loading groups...
    uint32 group_count = 0;
    GroupInfo* group_member;

    do
    {
        Field* fields = result_data->Fetch();

        //Load group member data
        uint32 groupId = fields[0].GetUInt32();
        uint32 leaderGUID = fields[1].GetUInt32();
        uint8  groupType = fields[2].GetUInt8(); 

        group_member                        = new GroupInfo;
        group_member->leaderGUID            = leaderGUID;
        group_member->groupType             = groupType;        

        // check data correctness
        if (guidSet.find(group_member->leaderGUID) == guidSet.end())
        {
            sLog->outErrorDb("creature_groups table leader guid %u incorrect (not exist)", group_member->leaderGUID);
            delete group_member;
            return;
        }

        CreatureGroupMap[groupId] = group_member;

        sLog->outDebug(LOG_FILTER_UNITS, "CreatureGroup::LoadCreatureGroups: Load Group %u with Leader %u and groupType %u.", groupId, leaderGUID, groupType);
        ++group_count;
    }
    while (result_data->NextRow()) ;

    //Loading member...
    uint32 member_count = 0;
    do
    {
        Field* fields = result_member->Fetch();
        
        //Load group member data
        uint32 groupId = fields[0].GetUInt32();
        uint32 memberGUID = fields[1].GetUInt32();

        // check data correctness
        if (guidSet.find(memberGUID) == guidSet.end())
        {
            sLog->outErrorDb("creature_group_data table member guid %u incorrect (not exist)", memberGUID);
            continue;
        }

        CreatureGroupDataMap[memberGUID] = groupId;

        sLog->outDebug(LOG_FILTER_UNITS, "CreatureGroup::LoadCreatureGroups: Load Member %u for Group with groupId %u.", memberGUID, groupId);
        ++member_count;
    }
    while (result->NextRow()) ;

    sLog->outString(">> Loaded %u creatures in %u groups in %u ms", member_count, group_count, GetMSTimeDiffToNow(oldMSTime));
    sLog->outString();
}

void CreatureGroup::AddMember(Creature* member)
{
    if (!member)
        return;

    sLog->outDebug(LOG_FILTER_UNITS, "CreatureGroup::AddMember: Adding unit GUID: %u to group.", member->GetGUIDLow());

    m_members[member] = CreatureGroupMap.find(m_groupID)->second;
    member->SetGroup(this);
}

void CreatureGroup::RemoveMember(Creature* member)
{
    if (!member)
        return;

    m_members.erase(member);
    member->SetGroup(NULL);
}

void CreatureGroup::MemberAttackStart(Creature* member, Unit* target)
{
    for (CreatureGroupMemberType::iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
    {
        Creature* creature = itr->first;

        sLog->outDebug(LOG_FILTER_UNITS, "CreatureGroup::MemberAttackStart: group member instanceId %u .",member->GetInstanceId());

        //Skip one check
        if (creature == member)
            continue;

        if (!creature->isAlive())
            continue;

        if (creature->getVictim())
            continue;

        if (creature->IsAIEnabled)
            if (creature->IsValidAttackTarget(target))
                creature->AI()->AttackStart(target);
    }
}

bool CreatureGroup::IsAllowedToRespawn(Creature* member)
{
    uint8 groupType = CreatureGroupMap[m_groupID]->groupType;

    if (!member->GetMap()->IsRaid() || groupType == 0)
        return false;

    bool exist = false;
    //Check if at least one groupmember lives if no -> no respawn.
    for (CreatureGroupMemberType::iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
        if (itr->first->isAlive())
            exist = true;

    //Check if an groupmember is in combat, if yes -> no respawn.
    for (CreatureGroupMemberType::iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
       if (itr->first->isInCombat())
            exist = false;
    
    if (exist)
         sLog->outDebug(LOG_FILTER_UNITS, "CreatureGroup::IsAllowedToRespawn: group member instanceId %u can respawn.",member->GetInstanceId());

    return exist;
}
